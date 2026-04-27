#ifndef TESTREPORTGENERATOR_H
#define TESTREPORTGENERATOR_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QMap>

namespace Infrastructure {
namespace Reporting {

/**
 * @brief Individual test case result
 */
struct TestCaseResult {
    QString name;
    QString className;
    QString category;
    bool passed = false;
    qint64 durationMs = 0;
    QString errorMessage;
    QString stdoutLog;
    QDateTime timestamp;

    QJsonObject toJson() const;
};

/**
 * @brief Test suite execution summary
 */
struct TestSuiteSummary {
    QString suiteName;
    QDateTime startTime;
    QDateTime endTime;
    qint64 totalDurationMs = 0;
    int totalTests = 0;
    int passedTests = 0;
    int failedTests = 0;
    int skippedTests = 0;
    QMap<QString, int> categoryCounts;

    QJsonObject toJson() const;
};

/**
 * @brief Comprehensive test report generator
 *
 * Generates test reports in multiple formats:
 * - JSON: Machine-readable for CI integration
 * - HTML: Human-readable with styling
 * - JUnit XML: Compatible with Jenkins, GitHub Actions, etc.
 * - Markdown: Simple text reports
 *
 * All methods are Q_INVOKABLE for QML/JS access.
 */
class TestReportGenerator : public QObject {
    Q_OBJECT

public:
    explicit TestReportGenerator(QObject* parent = nullptr);

    /**
     * @brief Add a test case result
     */
    Q_INVOKABLE void addTestResult(const TestCaseResult& result);

    /**
     * @brief Add multiple test results from JSON array
     */
    Q_INVOKABLE void addResultsFromJson(const QJsonArray& results);

    /**
     * @brief Set suite summary information
     */
    Q_INVOKABLE void setSuiteSummary(const TestSuiteSummary& summary);

    /**
     * @brief Generate JSON report
     */
    Q_INVOKABLE bool generateJsonReport(const QString& filePath);

    /**
     * @brief Generate HTML report with styling
     */
    Q_INVOKABLE bool generateHtmlReport(const QString& filePath);

    /**
     * @brief Generate JUnit XML report for CI integration
     */
    Q_INVOKABLE bool generateJUnitXmlReport(const QString& filePath);

    /**
     * @brief Generate Markdown report
     */
    Q_INVOKABLE bool generateMarkdownReport(const QString& filePath);

    /**
     * @brief Get report data as JSON object
     */
    Q_INVOKABLE QJsonObject toJson() const;

    /**
     * @brief Get report data as HTML string
     */
    Q_INVOKABLE QString toHtml() const;

    /**
     * @brief Clear all results
     */
    Q_INVOKABLE void clear();

    /**
     * @brief Get summary statistics
     */
    Q_INVOKABLE TestSuiteSummary summary() const { return m_summary; }

signals:
    void reportGenerated(const QString& format, const QString& filePath);
    void resultsChanged();

private:
    QVector<TestCaseResult> m_results;
    TestSuiteSummary m_summary;

    QString escapeXml(const QString& text) const;
    QString escapeHtml(const QString& text) const;
};

/**
 * @brief CTest output parser for automatic report generation
 */
class CTestReportParser : public QObject {
    Q_OBJECT

public:
    explicit CTestReportParser(QObject* parent = nullptr);

    /**
     * @brief Parse CTest output log
     */
    Q_INVOKABLE bool parseCTestLog(const QString& logFilePath);

    /**
     * @brief Parse multiple test executable outputs
     */
    Q_INVOKABLE bool parseTestOutputs(const QStringList& outputFiles);

    /**
     * @brief Get parsed results
     */
    Q_INVOKABLE QVector<TestCaseResult> results() const { return m_results; }

    /**
     * @brief Get generated generator with parsed data
     */
    Q_INVOKABLE TestReportGenerator* createReportGenerator();

private:
    QVector<TestCaseResult> m_results;
};

} // namespace Reporting
} // namespace Infrastructure

#endif // TESTREPORTGENERATOR_H
