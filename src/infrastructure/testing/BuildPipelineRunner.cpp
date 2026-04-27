#include "BuildPipelineRunner.h"
#include <QDir>
#include <QFileInfo>
#include <QDebug>

namespace Infrastructure {
namespace Testing {

BuildPipelineRunner::BuildPipelineRunner(QObject* parent)
    : QObject(parent)
{
}

void BuildPipelineRunner::setConfig(const PipelineConfig& config) {
    m_config = config;
}

QProcessEnvironment BuildPipelineRunner::buildEnvironment() const {
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

    if (!m_config.qtDir.isEmpty()) {
        env.insert("QTDIR", m_config.qtDir);
        QString cmakePrefix = QDir(m_config.qtDir).absoluteFilePath("lib/cmake");
        env.insert("Qt6_DIR", QDir(m_config.qtDir).absoluteFilePath("lib/cmake/Qt6"));
        env.insert("Qt6Core_DIR", QDir(m_config.qtDir).absoluteFilePath("lib/cmake/Qt6Core"));
        env.insert("Qt6Quick_DIR", QDir(m_config.qtDir).absoluteFilePath("lib/cmake/Qt6Quick"));
        env.insert("Qt6Test_DIR", QDir(m_config.qtDir).absoluteFilePath("lib/cmake/Qt6Test"));
        env.insert("Qt6SerialPort_DIR", QDir(m_config.qtDir).absoluteFilePath("lib/cmake/Qt6SerialPort"));

        QString path = env.value("PATH");
        QString binPath = QDir(m_config.qtDir).absoluteFilePath("bin");
        env.insert("PATH", binPath + ";" + path);
    }

    return env;
}

bool BuildPipelineRunner::runProcess(const QString& program,
                                      const QStringList& arguments,
                                      const QString& workingDir,
                                      const QProcessEnvironment& env,
                                      int timeoutMs,
                                      QString& output) {
    QProcess process;
    process.setWorkingDirectory(workingDir);
    process.setProcessEnvironment(env);

    process.start(program, arguments);
    if (!process.waitForStarted(30000)) {
        m_lastError = QStringLiteral("Failed to start %1: %2").arg(program, process.errorString());
        return false;
    }

    bool finished = process.waitForFinished(timeoutMs);
    output = QString::fromLocal8Bit(process.readAllStandardOutput());
    QString errOutput = QString::fromLocal8Bit(process.readAllStandardError());
    if (!errOutput.isEmpty()) {
        output += "\n" + errOutput;
    }

    emit outputAvailable(program, output);

    if (!finished) {
        process.kill();
        m_lastError = QStringLiteral("%1 timed out after %2 ms").arg(program).arg(timeoutMs);
        return false;
    }

    return process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0;
}

bool BuildPipelineRunner::runConfigure(QString& output) {
    emit stageStarted("configure");

    QDir buildDir(m_config.buildDir);
    if (!buildDir.exists()) {
        buildDir.mkpath(".");
    }

    if (m_config.cleanBuild) {
        // Clean build directory
        QDir dir(m_config.buildDir);
        dir.removeRecursively();
        dir.mkpath(".");
    }

    QStringList args;
    args << "-S" << m_config.sourceDir;
    args << "-B" << m_config.buildDir;
    args << "-G" << m_config.cmakeGenerator;
    args << "-DCMAKE_BUILD_TYPE=" + m_config.buildType;

    if (!m_config.qtDir.isEmpty()) {
        args << "-DCMAKE_PREFIX_PATH=" + m_config.qtDir;
    }

    if (!m_config.cmakeToolchainFile.isEmpty()) {
        args << "-DCMAKE_TOOLCHAIN_FILE=" + m_config.cmakeToolchainFile;
    }

    auto env = buildEnvironment();
    bool ok = runProcess("cmake", args, m_config.buildDir, env, m_config.buildTimeoutMs, output);

    emit stageFinished("configure", ok, 0);
    return ok;
}

bool BuildPipelineRunner::runBuild(QString& output) {
    emit stageStarted("build");

    QStringList args;
    args << "--build" << m_config.buildDir;
    args << "--config" << m_config.buildType;
    args << "--parallel";

    auto env = buildEnvironment();
    bool ok = runProcess("cmake", args, m_config.buildDir, env, m_config.buildTimeoutMs, output);

    emit stageFinished("build", ok, 0);
    return ok;
}

bool BuildPipelineRunner::runTests(QString& output) {
    emit stageStarted("test");

    QStringList args;
    args << "--build" << m_config.buildDir;
    args << "--target" << "test";
    args << "--config" << m_config.buildType;

    auto env = buildEnvironment();
    env.insert("QT_QPA_PLATFORM", "offscreen");
    env.insert("QT_QUICK_CONTROLS_STYLE", "Basic");

    bool ok = runProcess("cmake", args, m_config.buildDir, env, m_config.testTimeoutMs, output);

    emit stageFinished("test", ok, 0);
    return ok;
}

BuildPipelineRunner::PipelineResult BuildPipelineRunner::runPipeline() {
    PipelineResult result;
    QElapsedTimer totalTimer;
    totalTimer.start();

    // Stage 1: Configure
    QString configureOutput;
    QElapsedTimer stageTimer;
    stageTimer.start();

    if (!runConfigure(configureOutput)) {
        result.failureStage = "configure";
        result.failureReason = m_lastError;
        result.configureDurationMs = stageTimer.elapsed();
        result.totalDurationMs = totalTimer.elapsed();
        result.buildOutput = configureOutput;
        emit pipelineFinished(result);
        return result;
    }
    result.configureDurationMs = stageTimer.elapsed();

    // Stage 2: Build
    QString buildOutput;
    stageTimer.restart();

    if (!runBuild(buildOutput)) {
        result.failureStage = "build";
        result.failureReason = m_lastError;
        result.buildDurationMs = stageTimer.elapsed();
        result.totalDurationMs = totalTimer.elapsed();
        result.buildOutput = configureOutput + "\n" + buildOutput;
        emit pipelineFinished(result);
        return result;
    }
    result.buildDurationMs = stageTimer.elapsed();

    // Stage 3: Test
    if (m_config.runTests) {
        QString testOutput;
        stageTimer.restart();

        if (!runTests(testOutput)) {
            result.failureStage = "test";
            result.failureReason = m_lastError;
            result.testDurationMs = stageTimer.elapsed();
            result.totalDurationMs = totalTimer.elapsed();
            result.buildOutput = configureOutput + "\n" + buildOutput;
            result.testOutput = testOutput;
            emit pipelineFinished(result);
            return result;
        }
        result.testDurationMs = stageTimer.elapsed();
        result.testOutput = testOutput;

        // Parse test results from ctest output
        // Look for patterns like "X/Y Test #..."
        QRegularExpression testRe("(\\d+)/(\\d+)\\s+Test");
        QRegularExpressionMatchIterator it = testRe.globalMatch(testOutput);
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            if (match.hasMatch()) {
                result.testsPassed = match.captured(1).toInt();
                result.testsFailed = match.captured(2).toInt() - result.testsPassed;
            }
        }
    }

    // Stage 4: Generate reports
    if (m_config.generateReports) {
        QDir reportDir(m_config.reportOutputDir);
        if (!reportDir.exists()) {
            reportDir.mkpath(".");
        }
        result.reportPath = reportDir.absoluteFilePath("pipeline_report.json");

        QJsonObject report;
        report["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        report["success"] = true;
        report["configure_duration_ms"] = static_cast<int>(result.configureDurationMs);
        report["build_duration_ms"] = static_cast<int>(result.buildDurationMs);
        report["test_duration_ms"] = static_cast<int>(result.testDurationMs);
        report["total_duration_ms"] = static_cast<int>(totalTimer.elapsed());
        report["tests_passed"] = result.testsPassed;
        report["tests_failed"] = result.testsFailed;

        QFile file(result.reportPath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QJsonDocument doc(report);
            file.write(doc.toJson(QJsonDocument::Indented));
            file.close();
        }
    }

    result.success = true;
    result.totalDurationMs = totalTimer.elapsed();
    result.buildOutput = configureOutput + "\n" + buildOutput;

    emit pipelineFinished(result);
    return result;
}

QString BuildPipelineRunner::detectQtCMakeDir(const QString& qtRoot) {
    QDir dir(qtRoot);
    if (!dir.exists()) {
        return QString();
    }

    // Try common paths
    QStringList candidates = {
        "lib/cmake/Qt6",
        "lib/cmake",
    };

    for (const QString& candidate : candidates) {
        QDir subDir = QDir(dir.absoluteFilePath(candidate));
        if (subDir.exists("Qt6Config.cmake")) {
            return subDir.absolutePath();
        }
    }

    return QString();
}

QString BuildPipelineRunner::detectCMakeGenerator() {
#ifdef Q_OS_WIN
    // Check for Ninja
    QProcess ninjaCheck;
    ninjaCheck.start("where", QStringList() << "ninja");
    if (ninjaCheck.waitForFinished(5000) && ninjaCheck.exitCode() == 0) {
        return "Ninja";
    }

    // Check for MinGW make
    QProcess mingwCheck;
    mingwCheck.start("where", QStringList() << "mingw32-make");
    if (mingwCheck.waitForFinished(5000) && mingwCheck.exitCode() == 0) {
        return "MinGW Makefiles";
    }

    return "Ninja Multi-Config";
#else
    return "Ninja";
#endif
}

} // namespace Testing
} // namespace Infrastructure
