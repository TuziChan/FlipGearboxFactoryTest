#include "TeamExecutionMonitor.h"
#include <QDir>
#include <QJsonDocument>
#include <QTextStream>
#include <QDebug>

namespace Infrastructure {
namespace Testing {

TeamExecutionMonitor::TeamExecutionMonitor(QObject* parent)
    : QObject(parent)
    , m_sessionId(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"))
{
}

void TeamExecutionMonitor::setLogDirectory(const QString& dir) {
    QMutexLocker locker(&m_mutex);
    m_logDirectory = dir;
    ensureDirectory();
}

void TeamExecutionMonitor::setSessionId(const QString& sessionId) {
    QMutexLocker locker(&m_mutex);
    m_sessionId = sessionId;
}

QString TeamExecutionMonitor::logDirectory() const {
    QMutexLocker locker(&m_mutex);
    return m_logDirectory;
}

void TeamExecutionMonitor::ensureDirectory() const {
    if (!m_logDirectory.isEmpty()) {
        QDir dir(m_logDirectory);
        if (!dir.exists()) {
            dir.mkpath(".");
        }
    }
}

QString TeamExecutionMonitor::logFilePath() const {
    if (m_logDirectory.isEmpty()) {
        return QString();
    }
    return QDir(m_logDirectory).absoluteFilePath(QStringLiteral("team_execution_%1.jsonl").arg(m_sessionId));
}

bool TeamExecutionMonitor::appendEvent(const QString& eventType, const QJsonObject& data) {
    QMutexLocker locker(&m_mutex);

    if (m_logDirectory.isEmpty()) {
        m_lastError = QStringLiteral("Log directory not set");
        return false;
    }

    ensureDirectory();
    QString path = logFilePath();

    QJsonObject event;
    event["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    event["session_id"] = m_sessionId;
    event["event_type"] = eventType;
    event["data"] = data;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        m_lastError = QStringLiteral("Failed to open log file: %1").arg(path);
        return false;
    }

    QJsonDocument doc(event);
    file.write(doc.toJson(QJsonDocument::Compact));
    file.write("\n");
    file.close();

    emit eventLogged(eventType, event);
    return true;
}

void TeamExecutionMonitor::logTaskAssigned(const QString& taskId,
                                            const QString& taskTitle,
                                            const QString& assigneeRole,
                                            const QString& description) {
    QJsonObject data;
    data["task_id"] = taskId;
    data["task_title"] = taskTitle;
    data["assignee_role"] = assigneeRole;
    data["description"] = description;
    appendEvent("task_assigned", data);
}

void TeamExecutionMonitor::logTaskCompleted(const QString& taskId,
                                             const QString& assigneeRole,
                                             const QString& summary,
                                             qint64 durationMs) {
    QJsonObject data;
    data["task_id"] = taskId;
    data["assignee_role"] = assigneeRole;
    data["summary"] = summary;
    data["duration_ms"] = static_cast<int>(durationMs);
    appendEvent("task_completed", data);
}

void TeamExecutionMonitor::logCodeChange(const QString& authorRole,
                                          const QString& filePath,
                                          const QString& changeType,
                                          int linesAdded,
                                          int linesRemoved,
                                          const QString& description) {
    QJsonObject data;
    data["author_role"] = authorRole;
    data["file_path"] = filePath;
    data["change_type"] = changeType;
    data["lines_added"] = linesAdded;
    data["lines_removed"] = linesRemoved;
    data["description"] = description;
    appendEvent("code_change", data);
}

void TeamExecutionMonitor::logBuildEvent(const QString& stepName,
                                          bool success,
                                          const QString& output,
                                          qint64 durationMs) {
    QJsonObject data;
    data["step_name"] = stepName;
    data["success"] = success;
    data["output"] = output;
    data["duration_ms"] = static_cast<int>(durationMs);
    appendEvent("build_event", data);
}

void TeamExecutionMonitor::logTestEvent(const QString& testSuite,
                                         bool passed,
                                         int totalTests,
                                         int failedTests,
                                         const QString& reportPath) {
    QJsonObject data;
    data["test_suite"] = testSuite;
    data["passed"] = passed;
    data["total_tests"] = totalTests;
    data["failed_tests"] = failedTests;
    data["report_path"] = reportPath;
    appendEvent("test_event", data);
}

void TeamExecutionMonitor::logError(const QString& category,
                                     const QString& message,
                                     const QString& sourceFile,
                                     int lineNumber) {
    QJsonObject data;
    data["category"] = category;
    data["message"] = message;
    data["source_file"] = sourceFile;
    data["line_number"] = lineNumber;
    appendEvent("error", data);
}

void TeamExecutionMonitor::logMetric(const QString& metricName,
                                      double value,
                                      const QString& unit) {
    QJsonObject data;
    data["metric_name"] = metricName;
    data["value"] = value;
    data["unit"] = unit;
    appendEvent("metric", data);
}

void TeamExecutionMonitor::logDecision(const QString& decisionId,
                                        const QString& decision,
                                        const QString& rationale,
                                        const QStringList& alternatives) {
    QJsonObject data;
    data["decision_id"] = decisionId;
    data["decision"] = decision;
    data["rationale"] = rationale;
    QJsonArray altArray;
    for (const QString& alt : alternatives) {
        altArray.append(alt);
    }
    data["alternatives"] = altArray;
    appendEvent("decision", data);
}

QJsonArray TeamExecutionMonitor::queryEvents(const QString& eventType,
                                              const QDateTime& from,
                                              const QDateTime& to) const {
    QMutexLocker locker(&m_mutex);
    QJsonArray results;

    QString path = logFilePath();
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return results;
    }

    while (!file.atEnd()) {
        QByteArray line = file.readLine().trimmed();
        if (line.isEmpty()) continue;

        QJsonDocument doc = QJsonDocument::fromJson(line);
        if (doc.isNull() || !doc.isObject()) continue;

        QJsonObject obj = doc.object();
        QString ts = obj["timestamp"].toString();
        QDateTime eventTime = QDateTime::fromString(ts, Qt::ISODate);

        if (!eventType.isEmpty() && obj["event_type"].toString() != eventType) {
            continue;
        }
        if (from.isValid() && eventTime < from) continue;
        if (to.isValid() && eventTime > to) continue;

        results.append(obj);
    }

    file.close();
    return results;
}

QJsonObject TeamExecutionMonitor::getSessionSummary() const {
    QMutexLocker locker(&m_mutex);
    QJsonObject summary;
    summary["session_id"] = m_sessionId;
    summary["log_directory"] = m_logDirectory;

    // Count events by type
    QMap<QString, int> eventCounts;
    int totalTasks = 0;
    int completedTasks = 0;
    int buildFailures = 0;
    int testFailures = 0;
    int totalErrors = 0;

    QString path = logFilePath();
    QFile file(path);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        while (!file.atEnd()) {
            QByteArray line = file.readLine().trimmed();
            if (line.isEmpty()) continue;

            QJsonDocument doc = QJsonDocument::fromJson(line);
            if (doc.isNull() || !doc.isObject()) continue;

            QJsonObject obj = doc.object();
            QString type = obj["event_type"].toString();
            eventCounts[type]++;

            if (type == QLatin1String("task_assigned")) totalTasks++;
            if (type == QLatin1String("task_completed")) completedTasks++;
            if (type == QLatin1String("build_event") && !obj["data"].toObject()["success"].toBool()) {
                buildFailures++;
            }
            if (type == QLatin1String("test_event") && !obj["data"].toObject()["passed"].toBool()) {
                testFailures++;
            }
            if (type == QLatin1String("error")) totalErrors++;
        }
        file.close();
    }

    QJsonObject countsObj;
    for (auto it = eventCounts.begin(); it != eventCounts.end(); ++it) {
        countsObj[it.key()] = it.value();
    }
    summary["event_counts"] = countsObj;
    summary["total_tasks"] = totalTasks;
    summary["completed_tasks"] = completedTasks;
    summary["build_failures"] = buildFailures;
    summary["test_failures"] = testFailures;
    summary["total_errors"] = totalErrors;

    return summary;
}

bool TeamExecutionMonitor::generateSessionReport(const QString& outputPath) const {
    QMutexLocker locker(&m_mutex);

    QJsonObject report;
    report["session_id"] = m_sessionId;
    report["generated_at"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    report["summary"] = getSessionSummary();

    // Include all events
    report["events"] = queryEvents(QString(), QDateTime(), QDateTime());

    QDir dir = QFileInfo(outputPath).dir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QFile file(outputPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = QString("Failed to open report file: %1").arg(outputPath);
        return false;
    }

    QJsonDocument doc(report);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    qInfo() << "Team execution report saved to:" << outputPath;
    return true;
}

} // namespace Testing
} // namespace Infrastructure
