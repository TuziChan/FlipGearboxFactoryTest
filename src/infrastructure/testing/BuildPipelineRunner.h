#ifndef BUILDPIPELINERUNNER_H
#define BUILDPIPELINERUNNER_H

#include <QObject>
#include <QProcess>
#include <QJsonObject>
#include <QJsonArray>
#include <QElapsedTimer>
#include <QDateTime>
#include <QMap>

namespace Infrastructure {
namespace Testing {

/**
 * @brief Automated build and test pipeline runner
 *
 * Orchestrates:
 * 1. CMake configuration
 * 2. Build compilation
 * 3. Test execution
 * 4. Report generation
 * 5. Result notification
 *
 * Designed for CI/CD integration and AI team automation.
 */
class BuildPipelineRunner : public QObject {
    Q_OBJECT

public:
    explicit BuildPipelineRunner(QObject* parent = nullptr);

    struct PipelineConfig {
        QString sourceDir;
        QString buildDir;
        QString cmakeGenerator = "Ninja"; // or "MinGW Makefiles", "Visual Studio 17 2022"
        QString cmakeToolchainFile;       // Optional cross-compilation
        QString qtDir;                    // e.g. D:/Qt/6.11.0/mingw_64
        QString buildType = "Debug";      // Debug, Release, RelWithDebInfo
        bool cleanBuild = false;
        bool runTests = true;
        bool generateReports = true;
        QString reportOutputDir;
        int buildTimeoutMs = 300000;      // 5 minutes
        int testTimeoutMs = 300000;       // 5 minutes
    };

    struct PipelineResult {
        bool success = false;
        QString failureStage; // "configure", "build", "test"
        QString failureReason;
        qint64 totalDurationMs = 0;
        qint64 configureDurationMs = 0;
        qint64 buildDurationMs = 0;
        qint64 testDurationMs = 0;
        int testsPassed = 0;
        int testsFailed = 0;
        QString buildOutput;
        QString testOutput;
        QString reportPath;
    };

    void setConfig(const PipelineConfig& config);
    PipelineResult runPipeline();

    // Individual stages (for granular control)
    bool runConfigure(QString& output);
    bool runBuild(QString& output);
    bool runTests(QString& output);

    // Utility
    static QString detectQtCMakeDir(const QString& qtRoot);
    static QString detectCMakeGenerator();

    Q_INVOKABLE QString lastError() const { return m_lastError; }

signals:
    void stageStarted(const QString& stage);
    void stageFinished(const QString& stage, bool success, qint64 durationMs);
    void pipelineFinished(const PipelineResult& result);
    void outputAvailable(const QString& stage, const QString& output);

private:
    PipelineConfig m_config;
    QString m_lastError;

    bool runProcess(const QString& program,
                    const QStringList& arguments,
                    const QString& workingDir,
                    const QProcessEnvironment& env,
                    int timeoutMs,
                    QString& output);

    QProcessEnvironment buildEnvironment() const;
};

} // namespace Testing
} // namespace Infrastructure

#endif // BUILDPIPELINERUNNER_H
