#ifndef TESTORCHESTRATOR_H
#define TESTORCHESTRATOR_H

#include <QObject>
#include <QProcess>
#include <QJsonObject>
#include <QJsonArray>
#include <QElapsedTimer>
#include <QMap>
#include <memory>

namespace Infrastructure {
namespace Testing {

/**
 * @brief Result of a single test executable run
 */
struct TestExecutableResult {
    QString name;
    bool passed = false;
    qint64 durationMs = 0;
    QString output;
    QString errorOutput;
    int exitCode = -1;
    QString executablePath;
};

/**
 * @brief Aggregated test suite report
 */
struct AggregatedTestReport {
    QString suiteName;
    QDateTime startTime;
    QDateTime endTime;
    qint64 totalDurationMs = 0;
    int totalExecutables = 0;
    int passedExecutables = 0;
    int failedExecutables = 0;
    QVector<TestExecutableResult> results;

    QJsonObject toJson() const;
    QString toHtml() const;
    QString toMarkdown() const;
};

/**
 * @brief Unified test orchestrator
 *
 * Discovers, executes, and aggregates results from all project test executables.
 * Supports parallel execution, filtering, and multiple output formats.
 */
class TestOrchestrator : public QObject {
    Q_OBJECT

public:
    explicit TestOrchestrator(QObject* parent = nullptr);

    // Configuration
    void setBuildDirectory(const QString& buildDir);
    void setTestPath(const QString& testPath); // Environment PATH for Qt DLLs
    void setParallelExecution(bool enabled);
    void setTimeoutMs(int timeoutMs);

    // Test discovery
    void discoverTests(const QString& directory);
    void registerTestExecutable(const QString& name, const QString& path);
    QStringList registeredTests() const;

    // Execution
    AggregatedTestReport runAll(const QString& suiteName = "Full Test Suite");
    AggregatedTestReport runFiltered(const QStringList& testNames, const QString& suiteName);
    AggregatedTestReport runByCategory(const QString& category);

    // Report generation
    bool saveJsonReport(const AggregatedTestReport& report, const QString& filePath);
    bool saveHtmlReport(const AggregatedTestReport& report, const QString& filePath);
    bool saveJUnitXmlReport(const AggregatedTestReport& report, const QString& filePath);

    // CTest integration
    bool runCtest(const QString& buildDir);

    Q_INVOKABLE QString lastError() const { return m_lastError; }

signals:
    void testStarted(const QString& testName);
    void testFinished(const TestExecutableResult& result);
    void suiteStarted(const QString& suiteName);
    void suiteFinished(const AggregatedTestReport& report);
    void progressUpdated(int completed, int total);

private:
    QString m_buildDirectory;
    QString m_testPath;
    QString m_lastError;
    bool m_parallel = false;
    int m_timeoutMs = 120000; // 2 minutes default

    QMap<QString, QString> m_testExecutables; // name -> path

    TestExecutableResult runSingleTest(const QString& name, const QString& path);
    QString findExecutable(const QString& name) const;
};

} // namespace Testing
} // namespace Infrastructure

#endif // TESTORCHESTRATOR_H
