#ifndef AUTOTESTFRAMEWORK_H
#define AUTOTESTFRAMEWORK_H

#include <QObject>
#include <QTest>
#include <QSignalSpy>
#include <QElapsedTimer>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QStringList>
#include <QMap>
#include <QDebug>
#include <memory>

namespace Tests {
namespace Framework {

/**
 * @brief Test execution result for a single test function
 */
struct TestFunctionResult {
    QString name;
    bool passed = false;
    qint64 durationMs = 0;
    QString errorMessage;
};

/**
 * @brief Test execution result for a single test class
 */
struct TestClassResult {
    QString className;
    bool passed = false;
    qint64 durationMs = 0;
    int totalFunctions = 0;
    int passedFunctions = 0;
    int failedFunctions = 0;
    QVector<TestFunctionResult> functionResults;
};

/**
 * @brief Complete test suite execution report
 */
struct TestSuiteReport {
    QString suiteName;
    QDateTime startTime;
    QDateTime endTime;
    qint64 totalDurationMs = 0;
    int totalClasses = 0;
    int passedClasses = 0;
    int failedClasses = 0;
    int totalFunctions = 0;
    int passedFunctions = 0;
    int failedFunctions = 0;
    QVector<TestClassResult> classResults;

    QJsonObject toJson() const;
    QString toHtml() const;
};

/**
 * @brief Base class for all automated tests in the project
 *
 * Provides:
 * - Automatic test timing
 * - Structured result collection
 * - JSON/HTML report generation
 * - Signal spy helpers
 */
class AutoTestBase : public QObject {
    Q_OBJECT

public:
    explicit AutoTestBase(QObject* parent = nullptr);

    /**
     * @brief Get the test class result after execution
     */
    TestClassResult testResult() const { return m_result; }

    /**
     * @brief Set test metadata
     */
    void setTestMetadata(const QString& category, const QString& description);

signals:
    void testProgress(const QString& testName, bool passed, qint64 durationMs);

protected:
    /**
     * @brief Time a block of code and record result
     */
    void recordTestResult(const QString& testName, bool passed, const QString& error = QString());

    /**
     * @brief Wait for a signal with timeout
     */
    bool waitForSignal(QObject* obj, const char* signal, int timeoutMs = 5000);

    /**
     * @brief Verify signal spy count
     */
    bool verifySignalCount(QSignalSpy& spy, int expectedCount, const QString& message = QString());

    TestClassResult m_result;
    QElapsedTimer m_testTimer;

private:
    QString m_category;
    QString m_description;
};

/**
 * @brief Automatic test discovery and execution engine
 *
 * Scans for QObject-based test classes and runs them,
 * collecting results into a unified report.
 */
class AutoTestRunner : public QObject {
    Q_OBJECT

public:
    explicit AutoTestRunner(QObject* parent = nullptr);

    /**
     * @brief Register a test object to be executed
     */
    void registerTest(QObject* testObject);

    /**
     * @brief Register multiple test objects
     */
    template<typename... Args>
    void registerTests(Args... args) {
        (registerTest(args), ...);
    }

    /**
     * @brief Run all registered tests
     */
    TestSuiteReport runAll(const QString& suiteName = "AutoTest Suite");

    /**
     * @brief Run tests filtered by category
     */
    TestSuiteReport runByCategory(const QString& category);

    /**
     * @brief Save report to JSON file
     */
    bool saveJsonReport(const TestSuiteReport& report, const QString& filePath);

    /**
     * @brief Save report to HTML file
     */
    bool saveHtmlReport(const TestSuiteReport& report, const QString& filePath);

signals:
    void suiteStarted(const QString& suiteName);
    void suiteFinished(const TestSuiteReport& report);
    void classStarted(const QString& className);
    void classFinished(const TestClassResult& result);

private:
    QVector<QObject*> m_testObjects;

    TestClassResult executeTestObject(QObject* testObject);
    bool runQTest(QObject* testObject, TestClassResult& result);
};

/**
 * @brief Performance measurement helper
 */
class PerformanceMetrics {
public:
    struct Measurement {
        QString name;
        qint64 durationMs;
        qint64 memoryBeforeKb;
        qint64 memoryAfterKb;
    };

    static Measurement measure(const QString& name, std::function<void()> operation);
    static qint64 currentMemoryUsageKb();
};

} // namespace Framework
} // namespace Tests

#endif // AUTOTESTFRAMEWORK_H
