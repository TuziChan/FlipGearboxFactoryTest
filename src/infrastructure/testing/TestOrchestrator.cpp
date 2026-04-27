#include "TestOrchestrator.h"
#include <QProcess>
#include <QFileInfo>
#include <QDir>
#include <QJsonDocument>
#include <QTextStream>
#include <QRegularExpression>
#include <QDebug>

namespace Infrastructure {
namespace Testing {

QJsonObject AggregatedTestReport::toJson() const {
    QJsonObject obj;
    obj["suite_name"] = suiteName;
    obj["start_time"] = startTime.toString(Qt::ISODate);
    obj["end_time"] = endTime.toString(Qt::ISODate);
    obj["total_duration_ms"] = static_cast<int>(totalDurationMs);

    QJsonObject summary;
    summary["total_executables"] = totalExecutables;
    summary["passed_executables"] = passedExecutables;
    summary["failed_executables"] = failedExecutables;
    summary["pass_rate"] = totalExecutables > 0
        ? static_cast<double>(passedExecutables) / totalExecutables
        : 0.0;
    obj["summary"] = summary;

    QJsonArray resultsArray;
    for (const auto& r : results) {
        QJsonObject rObj;
        rObj["name"] = r.name;
        rObj["passed"] = r.passed;
        rObj["duration_ms"] = static_cast<int>(r.durationMs);
        rObj["exit_code"] = r.exitCode;
        rObj["executable_path"] = r.executablePath;
        if (!r.errorOutput.isEmpty()) {
            rObj["error_output"] = r.errorOutput;
        }
        resultsArray.append(rObj);
    }
    obj["results"] = resultsArray;

    return obj;
}

QString AggregatedTestReport::toHtml() const {
    QString html;
    html += "<!DOCTYPE html>\n<html><head><meta charset=\"UTF-8\">\n";
    html += "<title>" + suiteName + " - Test Report</title>\n";
    html += "<style>";
    html += "body{font-family:system-ui,-apple-system,sans-serif;margin:20px;background:#f5f5f5;}";
    html += ".container{max-width:1200px;margin:0 auto;background:#fff;padding:20px;border-radius:8px;box-shadow:0 2px 4px rgba(0,0,0,0.1);}";
    html += "h1{color:#333;border-bottom:2px solid #e0e0e0;padding-bottom:10px;}";
    html += ".summary{display:grid;grid-template-columns:repeat(4,1fr);gap:15px;margin:20px 0;}";
    html += ".card{background:#f8f9fa;padding:15px;border-radius:6px;text-align:center;}";
    html += ".card h3{margin:0 0 10px 0;color:#666;font-size:14px;text-transform:uppercase;}";
    html += ".card .value{font-size:28px;font-weight:bold;color:#333;}";
    html += ".pass{color:#28a745;}.fail{color:#dc3545;}.warn{color:#ffc107;}";
    html += "table{width:100%;border-collapse:collapse;margin:20px 0;}";
    html += "th{background:#f8f9fa;padding:12px;text-align:left;font-weight:600;color:#555;border-bottom:2px solid #dee2e6;}";
    html += "td{padding:10px 12px;border-bottom:1px solid #e9ecef;}";
    html += "tr:hover{background:#f8f9fa;}";
    html += ".status-badge{display:inline-block;padding:4px 10px;border-radius:4px;font-size:12px;font-weight:600;}";
    html += ".status-pass{background:#d4edda;color:#155724;}";
    html += ".status-fail{background:#f8d7da;color:#721c24;}";
    html += ".timestamp{color:#888;font-size:14px;}";
    html += "</style></head><body>";

    html += "<div class=\"container\">";
    html += "<h1>" + suiteName + " - Test Report</h1>";
    html += "<p class=\"timestamp\">Start: " + startTime.toString(Qt::ISODate) +
            " | Duration: " + QString::number(totalDurationMs) + " ms</p>";

    double passRate = totalExecutables > 0
        ? (static_cast<double>(passedExecutables) / totalExecutables * 100.0)
        : 0.0;

    html += "<div class=\"summary\">";
    html += "<div class=\"card\"><h3>Total</h3><div class=\"value\">" + QString::number(totalExecutables) + "</div></div>";
    html += "<div class=\"card\"><h3>Passed</h3><div class=\"value pass\">" + QString::number(passedExecutables) + "</div></div>";
    html += "<div class=\"card\"><h3>Failed</h3><div class=\"value fail\">" + QString::number(failedExecutables) + "</div></div>";
    html += "<div class=\"card\"><h3>Pass Rate</h3><div class=\"value\">" + QString::number(passRate, 'f', 1) + "%</div></div>";
    html += "</div>";

    html += "<h2>Detailed Results</h2>";
    html += "<table><tr><th>Test</th><th>Status</th><th>Duration (ms)</th><th>Exit Code</th></tr>";
    for (const auto& r : results) {
        QString statusClass = r.passed ? "status-pass" : "status-fail";
        QString statusText = r.passed ? "PASS" : "FAIL";
        html += "<tr><td>" + r.name + "</td>";
        html += "<td><span class=\"status-badge " + statusClass + "\">" + statusText + "</span></td>";
        html += "<td>" + QString::number(r.durationMs) + "</td>";
        html += "<td>" + QString::number(r.exitCode) + "</td></tr>";
    }
    html += "</table></div></body></html>";

    return html;
}

QString AggregatedTestReport::toMarkdown() const {
    QString md;
    md += "# " + suiteName + "\n\n";
    md += "**Started:** " + startTime.toString(Qt::ISODate) + "\n";
    md += "**Duration:** " + QString::number(totalDurationMs) + " ms\n\n";

    double passRate = totalExecutables > 0
        ? (static_cast<double>(passedExecutables) / totalExecutables * 100.0)
        : 0.0;

    md += "## Summary\n\n";
    md += "| Metric | Value |\n";
    md += "|--------|-------|\n";
    md += "| Total | " + QString::number(totalExecutables) + " |\n";
    md += "| Passed | " + QString::number(passedExecutables) + " |\n";
    md += "| Failed | " + QString::number(failedExecutables) + " |\n";
    md += "| Pass Rate | " + QString::number(passRate, 'f', 1) + "% |\n\n";

    md += "## Results\n\n";
    md += "| Test | Status | Duration (ms) | Exit Code |\n";
    md += "|------|--------|---------------|-----------|\n";
    for (const auto& r : results) {
        QString status = r.passed ? "PASS" : "FAIL";
        md += "| " + r.name + " | " + status + " | " + QString::number(r.durationMs) + " | " + QString::number(r.exitCode) + " |\n";
    }
    md += "\n";
    return md;
}

// ============================================================================
// TestOrchestrator
// ============================================================================

TestOrchestrator::TestOrchestrator(QObject* parent)
    : QObject(parent)
{
}

void TestOrchestrator::setBuildDirectory(const QString& buildDir) {
    m_buildDirectory = buildDir;
}

void TestOrchestrator::setTestPath(const QString& testPath) {
    m_testPath = testPath;
}

void TestOrchestrator::setParallelExecution(bool enabled) {
    m_parallel = enabled;
}

void TestOrchestrator::setTimeoutMs(int timeoutMs) {
    m_timeoutMs = timeoutMs;
}

void TestOrchestrator::discoverTests(const QString& directory) {
    QDir dir(directory);
    if (!dir.exists()) {
        m_lastError = QStringLiteral("Directory not found: %1").arg(directory);
        return;
    }

    QStringList filters;
#ifdef Q_OS_WIN
    filters << "*Tests.exe" << "*Test.exe";
#else
    filters << "*Tests" << "*Test";
#endif

    QFileInfoList files = dir.entryInfoList(filters, QDir::Files | QDir::Executable);
    for (const auto& fi : files) {
        QString name = fi.completeBaseName();
        m_testExecutables[name] = fi.absoluteFilePath();
    }
}

void TestOrchestrator::registerTestExecutable(const QString& name, const QString& path) {
    m_testExecutables[name] = path;
}

QStringList TestOrchestrator::registeredTests() const {
    return m_testExecutables.keys();
}

AggregatedTestReport TestOrchestrator::runAll(const QString& suiteName) {
    return runFiltered(m_testExecutables.keys(), suiteName);
}

AggregatedTestReport TestOrchestrator::runFiltered(const QStringList& testNames, const QString& suiteName) {
    AggregatedTestReport report;
    report.suiteName = suiteName;
    report.startTime = QDateTime::currentDateTime();

    emit suiteStarted(suiteName);

    QElapsedTimer totalTimer;
    totalTimer.start();

    int completed = 0;
    int total = testNames.size();

    for (const QString& name : testNames) {
        QString path = m_testExecutables.value(name);
        if (path.isEmpty()) {
            TestExecutableResult result;
            result.name = name;
            result.passed = false;
            result.errorOutput = QStringLiteral("Test executable not registered");
            report.results.append(result);
            report.failedExecutables++;
            completed++;
            emit progressUpdated(completed, total);
            continue;
        }

        emit testStarted(name);
        auto result = runSingleTest(name, path);
        emit testFinished(result);

        report.results.append(result);
        report.totalExecutables++;
        if (result.passed) {
            report.passedExecutables++;
        } else {
            report.failedExecutables++;
        }

        completed++;
        emit progressUpdated(completed, total);
    }

    report.endTime = QDateTime::currentDateTime();
    report.totalDurationMs = totalTimer.elapsed();

    emit suiteFinished(report);
    return report;
}

AggregatedTestReport TestOrchestrator::runByCategory(const QString& category) {
    QStringList filtered;
    for (auto it = m_testExecutables.begin(); it != m_testExecutables.end(); ++it) {
        if (it.key().toLower().contains(category.toLower())) {
            filtered.append(it.key());
        }
    }
    return runFiltered(filtered, QStringLiteral("Category: %1").arg(category));
}

TestExecutableResult TestOrchestrator::runSingleTest(const QString& name, const QString& path) {
    TestExecutableResult result;
    result.name = name;
    result.executablePath = path;

    QElapsedTimer timer;
    timer.start();

    QProcess process;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("QT_QPA_PLATFORM", "offscreen");
    if (!m_testPath.isEmpty()) {
        QString pathEnv = env.value("PATH");
        env.insert("PATH", m_testPath + ";" + pathEnv);
    }
    process.setProcessEnvironment(env);
    process.setWorkingDirectory(m_buildDirectory);

    process.start(path, QStringList() << "-o" << "-" << "-txt");

    if (!process.waitForStarted(m_timeoutMs / 10)) {
        result.errorOutput = QStringLiteral("Failed to start: %1").arg(process.errorString());
        result.durationMs = timer.elapsed();
        return result;
    }

    bool finished = process.waitForFinished(m_timeoutMs);
    if (!finished) {
        process.kill();
        result.errorOutput = QStringLiteral("Test timed out after %1 ms").arg(m_timeoutMs);
        result.durationMs = timer.elapsed();
        return result;
    }

    result.durationMs = timer.elapsed();
    result.output = QString::fromLocal8Bit(process.readAllStandardOutput());
    result.errorOutput = QString::fromLocal8Bit(process.readAllStandardError());
    result.exitCode = process.exitCode();

    // Qt Test returns 0 on pass, non-zero on failure
    result.passed = (process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0);

    return result;
}

bool TestOrchestrator::runCtest(const QString& buildDir) {
    QProcess ctest;
    ctest.setWorkingDirectory(buildDir);
    ctest.start("ctest", QStringList() << "--output-on-failure" << "-C" << "Debug");
    if (!ctest.waitForStarted(10000)) {
        m_lastError = QStringLiteral("Failed to start ctest: %1").arg(ctest.errorString());
        return false;
    }
    if (!ctest.waitForFinished(300000)) { // 5 min
        ctest.kill();
        m_lastError = QStringLiteral("ctest timed out");
        return false;
    }
    return ctest.exitCode() == 0;
}

bool TestOrchestrator::saveJsonReport(const AggregatedTestReport& report, const QString& filePath) {
    QDir dir = QFileInfo(filePath).dir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = QStringLiteral("Failed to open report file: %1").arg(filePath);
        return false;
    }

    QJsonDocument doc(report.toJson());
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    qInfo() << "JSON report saved to:" << filePath;
    return true;
}

bool TestOrchestrator::saveHtmlReport(const AggregatedTestReport& report, const QString& filePath) {
    QDir dir = QFileInfo(filePath).dir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = QStringLiteral("Failed to open report file: %1").arg(filePath);
        return false;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    stream << report.toHtml();
    file.close();

    qInfo() << "HTML report saved to:" << filePath;
    return true;
}

bool TestOrchestrator::saveJUnitXmlReport(const AggregatedTestReport& report, const QString& filePath) {
    QDir dir = QFileInfo(filePath).dir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = QStringLiteral("Failed to open report file: %1").arg(filePath);
        return false;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);

    stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    stream << QStringLiteral("<testsuite name=\"%1\" tests=\"%2\" failures=\"%3\" time=\"%4\">\n")
              .arg(report.suiteName)
              .arg(report.totalExecutables)
              .arg(report.failedExecutables)
              .arg(report.totalDurationMs / 1000.0);

    for (const auto& r : report.results) {
        stream << QStringLiteral("  <testcase name=\"%1\" time=\"%2\">\n")
                  .arg(r.name)
                  .arg(r.durationMs / 1000.0);
        if (!r.passed) {
            stream << QStringLiteral("    <failure message=\"Exit code %1\">%2</failure>\n")
                      .arg(r.exitCode)
                      .arg(r.errorOutput.toHtmlEscaped());
        }
        stream << "  </testcase>\n";
    }

    stream << "</testsuite>\n";
    file.close();

    qInfo() << "JUnit XML report saved to:" << filePath;
    return true;
}

} // namespace Testing
} // namespace Infrastructure
