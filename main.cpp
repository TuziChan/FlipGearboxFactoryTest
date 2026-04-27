#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QFontDatabase>
#include <QFont>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QLoggingCategory>
#include <QMutex>
#include <QMutexLocker>
#include <exception>
#include <cstdio>

#ifdef Q_OS_WIN
#include <windows.h>
#include <dbghelp.h>
#endif

#include "src/infrastructure/config/StationConfig.h"
#include "src/infrastructure/config/ConfigLoader.h"
#include "src/infrastructure/config/DeviceConfigService.h"
#include "src/infrastructure/config/StationRuntimeFactory.h"
#include "src/infrastructure/config/RuntimeManager.h"
#include "src/infrastructure/config/RecipeConfig.h"
#include "src/infrastructure/services/RecipeService.h"
#include "src/infrastructure/services/HistoryService.h"
#include "src/viewmodels/TestExecutionViewModel.h"
#include "src/viewmodels/DiagnosticsViewModel.h"
#include "src/viewmodels/HistoryViewModel.h"
#include "src/viewmodels/RecipeViewModel.h"
#include "src/ui/ChartPainter.h"
#include "src/teamops/MockTeamDataProvider.h"
#include "src/teamops/FileTeamDataProvider.h"
#include "src/teamops/TeamMonitorService.h"
#include "src/teamops/TeamOpsViewModel.h"

namespace {
QMutex g_logMutex;
QString g_crashLogFilePath;
QString g_crashArtifactDir;

QString isoTimestamp()
{
    return QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
}

QString fileTimestamp()
{
    return QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss-zzz"));
}

void initializeCrashLogFile(const QString &path)
{
    QMutexLocker locker(&g_logMutex);
    g_crashLogFilePath = path;

    QFileInfo info(path);
    g_crashArtifactDir = info.absolutePath();
    QDir().mkpath(info.absolutePath());

    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        const QString banner = QStringLiteral("\n============================================================\n"
                                               "[%1] New application run\n"
                                               "============================================================\n")
                                   .arg(isoTimestamp());
        file.write(banner.toUtf8());
        file.flush();
    }
}

void appendToCrashLog(const QString &line)
{
    QMutexLocker locker(&g_logMutex);
    if (g_crashLogFilePath.isEmpty()) {
        return;
    }

    QFile file(g_crashLogFilePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        file.write(line.toUtf8());
        file.write("\n");
        file.flush();
    }
}

#ifdef Q_OS_WIN
bool writeMiniDump(EXCEPTION_POINTERS *exceptionInfo, const QString &sourceTag)
{
    QString dumpPath;
    {
        QMutexLocker locker(&g_logMutex);
        if (g_crashArtifactDir.isEmpty()) {
            return false;
        }
        QDir().mkpath(g_crashArtifactDir);
        dumpPath = QDir(g_crashArtifactDir).absoluteFilePath(
            QStringLiteral("startup-crash-%1-%2.dmp").arg(sourceTag, fileTimestamp()));
    }

    HANDLE dumpFile = CreateFileW(reinterpret_cast<LPCWSTR>(dumpPath.utf16()),
                                  GENERIC_WRITE,
                                  0,
                                  nullptr,
                                  CREATE_ALWAYS,
                                  FILE_ATTRIBUTE_NORMAL,
                                  nullptr);

    if (dumpFile == INVALID_HANDLE_VALUE) {
        appendToCrashLog(QStringLiteral("[%1] [Fatal] [CrashDump] Failed to create dump file: %2")
                             .arg(isoTimestamp(), dumpPath));
        return false;
    }

    MINIDUMP_EXCEPTION_INFORMATION exceptionInfoBlock;
    MINIDUMP_EXCEPTION_INFORMATION *exceptionInfoPtr = nullptr;
    if (exceptionInfo) {
        exceptionInfoBlock.ThreadId = GetCurrentThreadId();
        exceptionInfoBlock.ExceptionPointers = exceptionInfo;
        exceptionInfoBlock.ClientPointers = FALSE;
        exceptionInfoPtr = &exceptionInfoBlock;
    }

    const auto dumpType = static_cast<MINIDUMP_TYPE>(MiniDumpWithThreadInfo
                                                     | MiniDumpWithIndirectlyReferencedMemory
                                                     | MiniDumpScanMemory);
    const BOOL success = MiniDumpWriteDump(GetCurrentProcess(),
                                           GetCurrentProcessId(),
                                           dumpFile,
                                           dumpType,
                                           exceptionInfoPtr,
                                           nullptr,
                                           nullptr);
    const DWORD errorCode = success ? ERROR_SUCCESS : GetLastError();
    CloseHandle(dumpFile);

    appendToCrashLog(QStringLiteral("[%1] [%2] [CrashDump] %3 (%4)")
                         .arg(isoTimestamp(),
                              sourceTag,
                              success ? QStringLiteral("Dump written to %1").arg(dumpPath)
                                      : QStringLiteral("Dump failed, error=%1, path=%2").arg(errorCode).arg(dumpPath),
                              success ? QStringLiteral("OK") : QStringLiteral("FAILED")));
    return success == TRUE;
}

LONG WINAPI unhandledExceptionFilter(EXCEPTION_POINTERS *exceptionInfo)
{
    appendToCrashLog(QStringLiteral("[%1] [Fatal] [UnhandledException] Caught unhandled Windows exception")
                         .arg(isoTimestamp()));
    writeMiniDump(exceptionInfo, QStringLiteral("UnhandledException"));
    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

void terminateMessageHandler()
{
    const QString line = QStringLiteral("[%1] [Fatal] [Terminate] std::terminate invoked")
                             .arg(isoTimestamp());
    appendToCrashLog(line);
#ifdef Q_OS_WIN
    writeMiniDump(nullptr, QStringLiteral("Terminate"));
#endif
    std::fprintf(stderr, "%s\n", line.toLocal8Bit().constData());
    std::fflush(stderr);
    std::abort();
}
} // namespace

// Custom message handler to catch all Qt messages
void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString typeStr;
    switch (type) {
    case QtDebugMsg:
        typeStr = "Debug";
        break;
    case QtInfoMsg:
        typeStr = "Info";
        break;
    case QtWarningMsg:
        typeStr = "Warning";
        break;
    case QtCriticalMsg:
        typeStr = "Critical";
        break;
    case QtFatalMsg:
        typeStr = "Fatal";
        break;
    }

    QString contextStr;
    if (context.file) {
        contextStr = QString(" (%1:%2, %3)").arg(context.file).arg(context.line).arg(context.function);
    }

    const QString timestamp = QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
    const QString fullMsg = QString("[%1] [%2]%3 %4").arg(timestamp, typeStr, contextStr, msg);
    std::fprintf(stderr, "%s\n", fullMsg.toLocal8Bit().constData());
    std::fflush(stderr);
    appendToCrashLog(fullMsg);

    if (type == QtFatalMsg) {
#ifdef Q_OS_WIN
        writeMiniDump(nullptr, QStringLiteral("QtFatal"));
#endif
        std::abort();
    }
}

int main(int argc, char *argv[])
{
    const QString executablePath = QFileInfo(QString::fromLocal8Bit(argv[0])).isAbsolute()
                                       ? QFileInfo(QString::fromLocal8Bit(argv[0])).absoluteFilePath()
                                       : QDir::current().absoluteFilePath(QString::fromLocal8Bit(argv[0]));
    const QString crashLogPath = QFileInfo(executablePath).absoluteDir().absoluteFilePath("logs/startup-crash.log");
    initializeCrashLogFile(crashLogPath);

    // Install custom message handler FIRST
    qInstallMessageHandler(customMessageHandler);
    std::set_terminate(terminateMessageHandler);
#ifdef Q_OS_WIN
    SetUnhandledExceptionFilter(unhandledExceptionFilter);
#endif

    // Enable QML debugging categories
    QLoggingCategory::setFilterRules(QStringLiteral("qt.qml.*=true\nqt.quick.*=true"));

    const QStringList rawArgs = [&]() {
        QStringList values;
        for (int i = 0; i < argc; ++i) {
            values.append(QString::fromLocal8Bit(argv[i]));
        }
        return values;
    }();

    QString chartBackend = qEnvironmentVariable("FLIP_CHART_BACKEND");
    if (chartBackend.isEmpty())
        chartBackend = QStringLiteral("qtgraphs");
    for (const QString &arg : rawArgs) {
        if (arg.startsWith(QStringLiteral("--chart-backend="))) {
            chartBackend = arg.section('=', 1);
        }
    }
    chartBackend = chartBackend.trimmed().toLower();
    if (chartBackend == QStringLiteral("graphs"))
        chartBackend = QStringLiteral("qtgraphs");
    if (chartBackend != QStringLiteral("painter"))
        chartBackend = QStringLiteral("qtgraphs");

    QString chartRhiBackend = qEnvironmentVariable("FLIP_QSG_RHI_BACKEND");
    for (const QString &arg : rawArgs) {
        if (arg.startsWith(QStringLiteral("--chart-rhi-backend="))) {
            chartRhiBackend = arg.section('=', 1);
        }
    }
    chartRhiBackend = chartRhiBackend.trimmed().toLower();
    if (!chartRhiBackend.isEmpty() && qEnvironmentVariableIsEmpty("QSG_RHI_BACKEND")) {
        qputenv("QSG_RHI_BACKEND", chartRhiBackend.toUtf8());
    }

    qDebug() << "========================================";
    qDebug() << "APPLICATION STARTUP";
    qDebug() << "========================================";
    qDebug() << "[Startup] Crash log file:" << crashLogPath;
    qDebug() << "[Startup] Chart backend:" << chartBackend;
    qDebug() << "[Startup] QSG_RHI_BACKEND:" << qEnvironmentVariable("QSG_RHI_BACKEND");

    QQuickStyle::setStyle(QStringLiteral("Basic"));
    QGuiApplication app(argc, argv);

    qDebug() << "[Startup] QGuiApplication created";

    // Load HarmonyOS Sans fonts
    QFontDatabase::addApplicationFont("fonts/HarmonyOS_Sans_SC_Regular.ttf");
    QFontDatabase::addApplicationFont("fonts/HarmonyOS_Sans_SC_Medium.ttf");
    QFontDatabase::addApplicationFont("fonts/HarmonyOS_Sans_SC_Bold.ttf");
    QFontDatabase::addApplicationFont("fonts/HarmonyOS_Sans_SC_Light.ttf");

    // Set default application font
    QFont defaultFont("HarmonyOS Sans SC");
    defaultFont.setPixelSize(14);
    app.setFont(defaultFont);

    qDebug() << "[Startup] Fonts loaded";

    Infrastructure::Config::StationConfig stationConfig;

    const QString stationConfigPath = QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("../../config/station.json");
    qDebug() << "[Startup] Loading station config from:" << stationConfigPath;

    Infrastructure::Config::ConfigLoader configLoader;
    if (!configLoader.loadStationConfig(stationConfigPath, stationConfig)) {
        qCritical() << "========================================";
        qCritical() << "CONFIGURATION LOAD FAILURE";
        qCritical() << "========================================";
        qCritical() << "Failed to load station config from:" << stationConfigPath;
        qCritical() << "Error:" << configLoader.lastError();
        qCritical() << "";
        qCritical() << "Fallback Strategy: Using built-in default configuration";
        qCritical() << "WARNING: Default configuration may not match your hardware setup!";
        qCritical() << "Please verify config/station.json exists and is valid before production use.";
        qCritical() << "========================================";

        // Check if config file exists at all
        if (!QFile::exists(stationConfigPath)) {
            qCritical() << "Config file does not exist. Creating default config is recommended.";
        }

        // In production mode (no --mock flag), config failure should be more严格
        // For now, we continue with defaults but log prominently
    } else {
        qDebug() << "[Startup] Station config loaded successfully";
    }

    // Detect mock mode from command line
    const QStringList args = app.arguments();
    bool mockMode = args.contains("--mock");
    qDebug() << "[Startup] Mock mode:" << (mockMode ? "ENABLED" : "DISABLED");

    // Create runtime manager
    qDebug() << "[Startup] Creating RuntimeManager...";
    auto runtimeManager = new Infrastructure::Config::RuntimeManager(stationConfig, mockMode, &app);
    qDebug() << "[Startup] RuntimeManager created";

    // Create ViewModel
    qDebug() << "[Startup] Creating ViewModels...";
    auto viewModel = new ViewModels::TestExecutionViewModel(runtimeManager->runtime(), runtimeManager, &app);
    auto diagnosticsViewModel = new ViewModels::DiagnosticsViewModel(runtimeManager->runtime(), runtimeManager, &app);
    auto deviceConfigService = new Infrastructure::Config::DeviceConfigService(stationConfigPath, stationConfig, &app);

    // Create services (must be before ViewModels that depend on them)
    auto recipeService = new Infrastructure::Services::RecipeService(&app);
    auto historyService = new Infrastructure::Services::HistoryService(&app);

    auto historyViewModel = new ViewModels::HistoryViewModel(historyService, &app);
    auto recipeViewModel = new ViewModels::RecipeViewModel(recipeService, &app);
    qDebug() << "[Startup] ViewModels created";

    // Create TeamOps monitoring subsystem
    qDebug() << "[Startup] Creating TeamOps subsystem...";
    TeamOps::ITeamDataProvider* teamDataProvider = nullptr;

    // Try file-based provider first (for real-time external data injection)
    const QString teamStatusPath = QDir(QCoreApplication::applicationDirPath())
                                       .absoluteFilePath("../../data/team_status.json");
    if (QFile::exists(teamStatusPath)) {
        auto fileProvider = new TeamOps::FileTeamDataProvider(teamStatusPath, &app);
        if (fileProvider->isAvailable()) {
            teamDataProvider = fileProvider;
            qDebug() << "[TeamOps] Using FileTeamDataProvider:" << teamStatusPath;
        } else {
            delete fileProvider;
        }
    }

    // Fall back to mock provider if file not available
    if (!teamDataProvider) {
        teamDataProvider = new TeamOps::MockTeamDataProvider(&app);
        qDebug() << "[TeamOps] Using MockTeamDataProvider (fallback)";
    }

    auto teamMonitorService = new TeamOps::TeamMonitorService(teamDataProvider, 5000, &app);
    auto teamOpsViewModel = new TeamOps::TeamOpsViewModel(teamMonitorService, &app);
    qDebug() << "[Startup] TeamOps subsystem created";

    QObject::connect(&app, &QCoreApplication::aboutToQuit, &app, [teamMonitorService, runtimeManager]() {
        qDebug() << "[Shutdown] Application about to quit, cleaning up...";
        if (teamMonitorService) {
            teamMonitorService->stopMonitoring();
        }
        if (runtimeManager && runtimeManager->runtime()) {
            runtimeManager->runtime()->shutdown();
        }
        qDebug() << "[Shutdown] Cleanup complete";
    });

    // Start monitoring automatically
    teamMonitorService->startMonitoring();

    // Create QML engine
    qDebug() << "[Startup] Creating QML engine...";
    QQmlApplicationEngine engine;

    // Connect to QML warnings and errors BEFORE loading
    QObject::connect(&engine, &QQmlApplicationEngine::warnings, &app, [](const QList<QQmlError> &warnings) {
        qWarning() << "[QML] ========== QML WARNINGS DETECTED ==========";
        for (const auto &warning : warnings) {
            qWarning() << "[QML Warning]" << warning.toString();
            qWarning() << "  File:" << warning.url();
            qWarning() << "  Line:" << warning.line() << "Column:" << warning.column();
            qWarning() << "  Description:" << warning.description();
        }
        qWarning() << "[QML] ============================================";
    });

    // Register custom QML types
    qDebug() << "[Startup] Registering QML types...";
    qmlRegisterType<ChartPainter>("FlipGearboxFactoryTest.UI", 1, 0, "ChartPainter");
    qDebug() << "[Startup] QML types registered";

    // Expose ViewModel to QML
    qDebug() << "[Startup] Setting QML context properties...";
    engine.rootContext()->setContextProperty("stationRuntime", runtimeManager->runtime());
    engine.rootContext()->setContextProperty("runtimeManager", runtimeManager);
    engine.rootContext()->setContextProperty("testViewModel", viewModel);
    engine.rootContext()->setContextProperty("diagnosticsViewModel", diagnosticsViewModel);
    engine.rootContext()->setContextProperty("historyViewModel", historyViewModel);
    engine.rootContext()->setContextProperty("recipeViewModel", recipeViewModel);
    engine.rootContext()->setContextProperty("deviceConfigService", deviceConfigService);
    engine.rootContext()->setContextProperty("recipeService", recipeService);
    engine.rootContext()->setContextProperty("historyService", historyService);
    engine.rootContext()->setContextProperty("teamOpsViewModel", teamOpsViewModel);
    engine.rootContext()->setContextProperty("chartBackend", chartBackend);
    qDebug() << "[Startup] QML context properties set";

    // CRITICAL FIX: Update QML context property when runtime is recreated
    // This prevents dangling pointer access in QML after mode switching
    QObject::connect(runtimeManager, &Infrastructure::Config::RuntimeManager::runtimeRecreated,
                     &app, [&engine](Infrastructure::Config::StationRuntime* newRuntime) {
        qDebug() << "[main] Runtime recreated, updating QML context property to prevent dangling pointer...";
        engine.rootContext()->setContextProperty("stationRuntime", newRuntime);
        qDebug() << "[main] QML context updated successfully";
    });

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() {
            qCritical() << "[QML] ========== OBJECT CREATION FAILED ==========";
            qCritical() << "[QML] QML object creation failed! Application will exit.";
            qCritical() << "[QML] Check the warnings above for details.";
            qCritical() << "[QML] =============================================";
            QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);

    qDebug() << "[Startup] About to load QML from module: FlipGearboxFactoryTest/Main";
    qDebug() << "[Startup] QML import paths:" << engine.importPathList();
    qDebug() << "[Startup] QML plugin paths:" << engine.pluginPathList();

    try {
        qDebug() << "[Startup] Calling engine.loadFromModule()...";
        engine.loadFromModule("FlipGearboxFactoryTest", "Main");
        qDebug() << "[Startup] engine.loadFromModule() returned";
    } catch (const std::exception &e) {
        qCritical() << "[Startup] Exception caught during QML loading:" << e.what();
        return -1;
    } catch (...) {
        qCritical() << "[Startup] Unknown exception caught during QML loading";
        return -1;
    }

    qDebug() << "[Startup] Checking root objects...";
    if (engine.rootObjects().isEmpty()) {
        qCritical() << "[QML] No root objects created! QML loading failed.";
        qCritical() << "[QML] This usually means there was a QML error during loading.";
        return -1;
    }

    qDebug() << "[Startup] QML loaded successfully, root objects count:" << engine.rootObjects().size();
    qDebug() << "========================================";
    qDebug() << "APPLICATION STARTUP COMPLETE";
    qDebug() << "========================================";

    return QCoreApplication::exec();
}
