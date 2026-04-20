#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QFontDatabase>
#include <QFont>
#include <QDebug>
#include <QDir>

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

int main(int argc, char *argv[])
{
    QQuickStyle::setStyle(QStringLiteral("Basic"));
    QGuiApplication app(argc, argv);

    // Load HarmonyOS Sans fonts
    QFontDatabase::addApplicationFont("fonts/HarmonyOS_Sans_SC_Regular.ttf");
    QFontDatabase::addApplicationFont("fonts/HarmonyOS_Sans_SC_Medium.ttf");
    QFontDatabase::addApplicationFont("fonts/HarmonyOS_Sans_SC_Bold.ttf");
    QFontDatabase::addApplicationFont("fonts/HarmonyOS_Sans_SC_Light.ttf");

    // Set default application font
    QFont defaultFont("HarmonyOS Sans SC");
    defaultFont.setPixelSize(14);
    app.setFont(defaultFont);

    Infrastructure::Config::StationConfig stationConfig;

    const QString stationConfigPath = QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("../../config/station.json");
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
    }

    // Detect mock mode from command line
    const QStringList args = app.arguments();
    bool mockMode = args.contains("--mock");

    // Create runtime manager
    auto runtimeManager = new Infrastructure::Config::RuntimeManager(stationConfig, mockMode, &app);

    // Create ViewModel
    auto viewModel = new ViewModels::TestExecutionViewModel(runtimeManager->runtime(), runtimeManager, &app);
    auto diagnosticsViewModel = new ViewModels::DiagnosticsViewModel(runtimeManager->runtime(), runtimeManager, &app);
    auto deviceConfigService = new Infrastructure::Config::DeviceConfigService(stationConfigPath, stationConfig, &app);

    // Create services (must be before ViewModels that depend on them)
    auto recipeService = new Infrastructure::Services::RecipeService(&app);
    auto historyService = new Infrastructure::Services::HistoryService(&app);

    auto historyViewModel = new ViewModels::HistoryViewModel(historyService, &app);
    auto recipeViewModel = new ViewModels::RecipeViewModel(recipeService, &app);

    // Create QML engine
    QQmlApplicationEngine engine;

    // Expose ViewModel to QML
    engine.rootContext()->setContextProperty("stationRuntime", runtimeManager->runtime());
    engine.rootContext()->setContextProperty("runtimeManager", runtimeManager);
    engine.rootContext()->setContextProperty("testViewModel", viewModel);
    engine.rootContext()->setContextProperty("diagnosticsViewModel", diagnosticsViewModel);
    engine.rootContext()->setContextProperty("historyViewModel", historyViewModel);
    engine.rootContext()->setContextProperty("recipeViewModel", recipeViewModel);
    engine.rootContext()->setContextProperty("deviceConfigService", deviceConfigService);
    engine.rootContext()->setContextProperty("recipeService", recipeService);
    engine.rootContext()->setContextProperty("historyService", historyService);
    
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    
    engine.loadFromModule("FlipGearboxFactoryTest", "Main");

    int result = QCoreApplication::exec();

    // Cleanup
    runtimeManager->runtime()->shutdown();

    return result;
}
