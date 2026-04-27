#include "TestReportGenerator.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QJsonDocument>
#include <QDebug>

namespace Infrastructure {
namespace Reporting {

// ============================================================================
// TestCaseResult
// ============================================================================

QJsonObject TestCaseResult::toJson() const {
    QJsonObject obj;
    obj["name"] = name;
    obj["class_name"] = className;
    obj["category"] = category;
    obj["passed"] = passed;
    obj["duration_ms"] = static_cast<int>(durationMs);
    if (!errorMessage.isEmpty()) {
        obj["error"] = errorMessage;
    }
    if (!stdoutLog.isEmpty()) {
        obj["stdout"] = stdoutLog;
    }
    obj["timestamp"] = timestamp.toString(Qt::ISODate);
    return obj;
}

// ============================================================================
// TestSuiteSummary
// ============================================================================

QJsonObject TestSuiteSummary::toJson() const {
    QJsonObject obj;
    obj["suite_name"] = suiteName;
    obj["start_time"] = startTime.toString(Qt::ISODate);
    obj["end_time"] = endTime.toString(Qt::ISODate);
    obj["total_duration_ms"] = static_cast<int>(totalDurationMs);
    obj["total_tests"] = totalTests;
    obj["passed_tests"] = passedTests;
    obj["failed_tests"] = failedTests;
    obj["skipped_tests"] = skippedTests;

    QJsonObject categories;
    for (auto it = categoryCounts.begin(); it != categoryCounts.end(); ++it) {
        categories[it.key()] = it.value();
    }
    obj["category_counts"] = categories;

    return obj;
}

// ============================================================================
// TestReportGenerator
// ============================================================================

TestReportGenerator::TestReportGenerator(QObject* parent)
    : QObject(parent)
{
}

void TestReportGenerator::addTestResult(const TestCaseResult& result)
{
    m_results.append(result);
    emit resultsChanged();
}

void TestReportGenerator::addResultsFromJson(const QJsonArray& results)
{
    for (const auto& val : results) {
        QJsonObject obj = val.toObject();
        TestCaseResult result;
        result.name = obj["name"].toString();
        result.className = obj["class_name"].toString();
        result.category = obj["category"].toString();
        result.passed = obj["passed"].toBool();
        result.durationMs = obj["duration_ms"].toInt();
        result.errorMessage = obj["error"].toString();
        result.stdoutLog = obj["stdout"].toString();
        result.timestamp = QDateTime::fromString(obj["timestamp"].toString(), Qt::ISODate);
        m_results.append(result);
    }
    emit resultsChanged();
}

void TestReportGenerator::setSuiteSummary(const TestSuiteSummary& summary)
{
    m_summary = summary;
}

bool TestReportGenerator::generateJsonReport(const QString& filePath)
{
    QDir dir = QFileInfo(filePath).dir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "TestReportGenerator: Failed to open file:" << filePath;
        return false;
    }

    QJsonDocument doc(toJson());
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    emit reportGenerated("JSON", filePath);
    qInfo() << "TestReportGenerator: JSON report saved to" << filePath;
    return true;
}

bool TestReportGenerator::generateHtmlReport(const QString& filePath)
{
    QDir dir = QFileInfo(filePath).dir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "TestReportGenerator: Failed to open file:" << filePath;
        return false;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    stream << toHtml();
    file.close();

    emit reportGenerated("HTML", filePath);
    qInfo() << "TestReportGenerator: HTML report saved to" << filePath;
    return true;
}

bool TestReportGenerator::generateJUnitXmlReport(const QString& filePath)
{
    QDir dir = QFileInfo(filePath).dir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "TestReportGenerator: Failed to open file:" << filePath;
        return false;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);

    stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    stream << "<testsuites>\n";
    stream << "  <testsuite name=\"" << escapeXml(m_summary.suiteName) << "\"\n";
    stream << "             tests=\"" << m_summary.totalTests << "\"\n";
    stream << "             failures=\"" << m_summary.failedTests << "\"\n";
    stream << "             skipped=\"" << m_summary.skippedTests << "\"\n";
    stream << "             time=\"" << (m_summary.totalDurationMs / 1000.0) << "\"\n";
    stream << "             timestamp=\"" << m_summary.startTime.toString(Qt::ISODate) << "\">\n";

    for (const auto& result : m_results) {
        stream << "    <testcase name=\"" << escapeXml(result.name) << "\"\n";
        stream << "              classname=\"" << escapeXml(result.className) << "\"\n";
        stream << "              time=\"" << (result.durationMs / 1000.0) << "\">\n";

        if (!result.passed) {
            stream << "      <failure message=\"" << escapeXml(result.errorMessage) << "\">\n";
            stream << "        " << escapeXml(result.stdoutLog) << "\n";
            stream << "      </failure>\n";
        }

        stream << "    </testcase>\n";
    }

    stream << "  </testsuite>\n";
    stream << "</testsuites>\n";

    file.close();

    emit reportGenerated("JUnit XML", filePath);
    qInfo() << "TestReportGenerator: JUnit XML report saved to" << filePath;
    return true;
}

bool TestReportGenerator::generateMarkdownReport(const QString& filePath)
{
    QDir dir = QFileInfo(filePath).dir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "TestReportGenerator: Failed to open file:" << filePath;
        return false;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);

    stream << "# " << m_summary.suiteName << " - Test Report\n\n";
    stream << "**Generated:** " << QDateTime::currentDateTime().toString(Qt::ISODate) << "\n\n";

    // Summary table
    stream << "## Summary\n\n";
    stream << "| Metric | Value |\n";
    stream << "|--------|-------|\n";
    stream << "| Total Tests | " << m_summary.totalTests << " |\n";
    stream << "| Passed | " << m_summary.passedTests << " |\n";
    stream << "| Failed | " << m_summary.failedTests << " |\n";
    stream << "| Skipped | " << m_summary.skippedTests << " |\n";
    stream << "| Duration | " << (m_summary.totalDurationMs / 1000.0) << " s |\n\n";

    // Results table
    stream << "## Test Results\n\n";
    stream << "| Test | Category | Status | Duration (ms) |\n";
    stream << "|------|----------|--------|---------------|\n";

    for (const auto& result : m_results) {
        QString status = result.passed ? "PASS" : "FAIL";
        stream << "| " << result.name << " | " << result.category << " | " << status
               << " | " << result.durationMs << " |\n";
    }

    stream << "\n";
    file.close();

    emit reportGenerated("Markdown", filePath);
    qInfo() << "TestReportGenerator: Markdown report saved to" << filePath;
    return true;
}

QJsonObject TestReportGenerator::toJson() const {
    QJsonObject root;
    root["summary"] = m_summary.toJson();

    QJsonArray resultsArray;
    for (const auto& result : m_results) {
        resultsArray.append(result.toJson());
    }
    root["results"] = resultsArray;

    return root;
}

QString TestReportGenerator::toHtml() const {
    QString html;
    html += "<!DOCTYPE html>\n";
    html += "<html><head><meta charset=\"UTF-8\">\n";
    html += "<title>" + m_summary.suiteName + " - Test Report</title>\n";
    html += "<style>\n";
    html += "body{font-family:system-ui,-apple-system,sans-serif;margin:0;padding:20px;background:#f0f2f5;color:#333;}\n";
    html += ".container{max-width:1200px;margin:0 auto;}\n";
    html += ".header{background:#fff;padding:24px 32px;border-radius:8px;margin-bottom:20px;box-shadow:0 1px 3px rgba(0,0,0,0.1);}\n";
    html += ".header h1{margin:0 0 8px 0;font-size:24px;color:#1a1a1a;}\n";
    html += ".header .meta{color:#666;font-size:14px;}\n";
    html += ".summary-grid{display:grid;grid-template-columns:repeat(4,1fr);gap:16px;margin-bottom:20px;}\n";
    html += ".summary-card{background:#fff;padding:20px;border-radius:8px;text-align:center;box-shadow:0 1px 3px rgba(0,0,0,0.1);}\n";
    html += ".summary-card h3{margin:0 0 8px 0;font-size:12px;text-transform:uppercase;color:#666;letter-spacing:0.5px;}\n";
    html += ".summary-card .value{font-size:32px;font-weight:700;color:#1a1a1a;}\n";
    html += ".summary-card.pass .value{color:#22c55e;}\n";
    html += ".summary-card.fail .value{color:#ef4444;}\n";
    html += ".summary-card.warn .value{color:#f59e0b;}\n";
    html += ".content{background:#fff;padding:24px 32px;border-radius:8px;box-shadow:0 1px 3px rgba(0,0,0,0.1);}\n";
    html += "table{width:100%;border-collapse:collapse;}\n";
    html += "th{padding:12px 16px;text-align:left;font-size:12px;text-transform:uppercase;color:#666;border-bottom:2px solid #e5e7eb;font-weight:600;}\n";
    html += "td{padding:12px 16px;border-bottom:1px solid #f3f4f6;}\n";
    html += "tr:hover{background:#f9fafb;}\n";
    html += ".status{display:inline-flex;align-items:center;padding:4px 12px;border-radius:9999px;font-size:12px;font-weight:600;}\n";
    html += ".status-pass{background:#dcfce7;color:#166534;}\n";
    html += ".status-fail{background:#fee2e2;color:#991b1b;}\n";
    html += ".status-skip{background:#fef3c7;color:#92400e;}\n";
    html += ".error-msg{color:#ef4444;font-size:13px;margin-top:4px;}\n";
    html += ".progress-bar{height:8px;background:#e5e7eb;border-radius:4px;overflow:hidden;margin-top:8px;}\n";
    html += ".progress-fill{height:100%;background:#22c55e;border-radius:4px;transition:width 0.3s;}\n";
    html += "</style></head><body>\n";

    html += "<div class=\"container\">\n";

    // Header
    html += "<div class=\"header\">\n";
    html += "<h1>" + m_summary.suiteName + "</h1>\n";
    html += "<div class=\"meta\">Generated: " + QDateTime::currentDateTime().toString(Qt::ISODate)
            + " | Duration: " + QString::number(m_summary.totalDurationMs / 1000.0, 'f', 2) + " s</div>\n";
    html += "</div>\n";

    // Summary grid
    double passRate = m_summary.totalTests > 0
                          ? (100.0 * m_summary.passedTests / m_summary.totalTests)
                          : 0.0;

    html += "<div class=\"summary-grid\">\n";
    html += "<div class=\"summary-card\"><h3>Total Tests</h3><div class=\"value\">" + QString::number(m_summary.totalTests) + "</div></div>\n";
    html += "<div class=\"summary-card pass\"><h3>Passed</h3><div class=\"value\">" + QString::number(m_summary.passedTests) + "</div></div>\n";
    html += "<div class=\"summary-card fail\"><h3>Failed</h3><div class=\"value\">" + QString::number(m_summary.failedTests) + "</div></div>\n";
    html += "<div class=\"summary-card warn\"><h3>Pass Rate</h3><div class=\"value\">" + QString::number(passRate, 'f', 1) + "%</div></div>\n";
    html += "</div>\n";

    // Progress bar
    html += "<div class=\"content\">\n";
    html += "<div class=\"progress-bar\"><div class=\"progress-fill\" style=\"width:" + QString::number(passRate) + "%\"></div></div>\n";
    html += "</div>\n";

    // Results table
    html += "<div class=\"content\" style=\"margin-top:20px;\">\n";
    html += "<h2 style=\"margin-top:0;\">Test Results</h2>\n";
    html += "<table>\n";
    html += "<tr><th>Test Name</th><th>Category</th><th>Status</th><th>Duration (ms)</th></tr>\n";

    for (const auto& result : m_results) {
        QString statusClass = result.passed ? "status-pass" : "status-fail";
        QString statusText = result.passed ? "PASS" : "FAIL";
        html += "<tr>";
        html += "<td><strong>" + escapeHtml(result.name) + "</strong>";
        if (!result.errorMessage.isEmpty()) {
            html += "<div class=\"error-msg\">" + escapeHtml(result.errorMessage) + "</div>";
        }
        html += "</td>";
        html += "<td>" + escapeHtml(result.category) + "</td>";
        html += "<td><span class=\"status " + statusClass + "\">" + statusText + "</span></td>";
        html += "<td>" + QString::number(result.durationMs) + "</td>";
        html += "</tr>\n";
    }

    html += "</table>\n";
    html += "</div>\n";

    html += "</div></body></html>\n";

    return html;
}

void TestReportGenerator::clear()
{
    m_results.clear();
    m_summary = TestSuiteSummary();
    emit resultsChanged();
}

QString TestReportGenerator::escapeXml(const QString& text) const {
    QString result = text;
    result.replace("&", "&amp;");
    result.replace("<", "&lt;");
    result.replace(">", "&gt;");
    result.replace("\"", "&quot;");
    result.replace("'", "&apos;");
    return result;
}

QString TestReportGenerator::escapeHtml(const QString& text) const {
    QString result = text;
    result.replace("&", "&amp;");
    result.replace("<", "&lt;");
    result.replace(">", "&gt;");
    result.replace("\"", "&quot;");
    return result;
}

// ============================================================================
// CTestReportParser
// ============================================================================

CTestReportParser::CTestReportParser(QObject* parent)
    : QObject(parent)
{
}

bool CTestReportParser::parseCTestLog(const QString& logFilePath)
{
    QFile file(logFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "CTestReportParser: Failed to open log file:" << logFilePath;
        return false;
    }

    QTextStream stream(&file);
    QString currentTest;
    QString currentOutput;
    bool currentPassed = true;

    while (!stream.atEnd()) {
        QString line = stream.readLine();

        // Parse test start/end patterns
        if (line.contains("Test #")) {
            if (!currentTest.isEmpty()) {
                TestCaseResult result;
                result.name = currentTest;
                result.passed = currentPassed;
                result.stdoutLog = currentOutput;
                m_results.append(result);
            }
            currentTest = line.section(" ", -1).trimmed();
            currentOutput.clear();
            currentPassed = true;
        } else if (line.contains("***Failed") || line.contains("Failed")) {
            currentPassed = false;
        }

        currentOutput += line + "\n";
    }

    // Don't forget the last test
    if (!currentTest.isEmpty()) {
        TestCaseResult result;
        result.name = currentTest;
        result.passed = currentPassed;
        result.stdoutLog = currentOutput;
        m_results.append(result);
    }

    file.close();
    return true;
}

bool CTestReportParser::parseTestOutputs(const QStringList& outputFiles)
{
    for (const QString& filePath : outputFiles) {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            continue;
        }

        QTextStream stream(&file);
        QString output = stream.readAll();
        file.close();

        TestCaseResult result;
        result.name = QFileInfo(filePath).baseName();
        result.passed = !output.contains("FAIL") && !output.contains("failed");
        result.stdoutLog = output;
        m_results.append(result);
    }

    return !m_results.isEmpty();
}

TestReportGenerator* CTestReportParser::createReportGenerator()
{
    auto* generator = new TestReportGenerator(this);

    for (const auto& result : m_results) {
        generator->addTestResult(result);
    }

    // Compute summary
    TestSuiteSummary summary;
    summary.suiteName = "CTest Auto-Generated Report";
    summary.startTime = QDateTime::currentDateTime();
    summary.endTime = QDateTime::currentDateTime();
    summary.totalTests = m_results.size();

    for (const auto& result : m_results) {
        if (result.passed) {
            summary.passedTests++;
        } else {
            summary.failedTests++;
        }
    }

    generator->setSuiteSummary(summary);
    return generator;
}

} // namespace Reporting
} // namespace Infrastructure
