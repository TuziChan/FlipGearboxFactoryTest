#ifndef TEAMEXECUTIONMONITOR_H
#define TEAMEXECUTIONMONITOR_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QFile>
#include <QMutex>
#include <QMap>
#include <QVector>

namespace Infrastructure {
namespace Testing {

/**
 * @brief Monitors and logs AI team execution traces
 *
 * Provides structured logging for:
 * - Team member task assignments and completions
 * - Code changes (files modified, lines added/removed)
 * - Build and test results
 * - Performance metrics over time
 * - Error patterns and failure root causes
 *
 * Outputs to JSONL format for downstream analysis.
 */
class TeamExecutionMonitor : public QObject {
    Q_OBJECT

public:
    explicit TeamExecutionMonitor(QObject* parent = nullptr);

    // Configuration
    void setLogDirectory(const QString& dir);
    void setSessionId(const QString& sessionId);
    QString logDirectory() const;

    // Event logging
    void logTaskAssigned(const QString& taskId,
                         const QString& taskTitle,
                         const QString& assigneeRole,
                         const QString& description);

    void logTaskCompleted(const QString& taskId,
                          const QString& assigneeRole,
                          const QString& summary,
                          qint64 durationMs);

    void logCodeChange(const QString& authorRole,
                       const QString& filePath,
                       const QString& changeType, // "add", "modify", "delete"
                       int linesAdded,
                       int linesRemoved,
                       const QString& description);

    void logBuildEvent(const QString& stepName,
                       bool success,
                       const QString& output,
                       qint64 durationMs);

    void logTestEvent(const QString& testSuite,
                      bool passed,
                      int totalTests,
                      int failedTests,
                      const QString& reportPath);

    void logError(const QString& category,
                  const QString& message,
                  const QString& sourceFile,
                  int lineNumber);

    void logMetric(const QString& metricName,
                   double value,
                   const QString& unit);

    void logDecision(const QString& decisionId,
                     const QString& decision,
                     const QString& rationale,
                     const QStringList& alternatives);

    // Query
    QJsonArray queryEvents(const QString& eventType,
                           const QDateTime& from,
                           const QDateTime& to) const;

    QJsonObject getSessionSummary() const;

    // Report generation
    bool generateSessionReport(const QString& outputPath) const;

    Q_INVOKABLE QString lastError() const { return m_lastError; }

signals:
    void eventLogged(const QString& eventType, const QJsonObject& eventData);

private:
    QString m_logDirectory;
    QString m_sessionId;
    mutable QString m_lastError;
    mutable QMutex m_mutex;

    QString logFilePath() const;
    bool appendEvent(const QString& eventType, const QJsonObject& data);
    void ensureDirectory() const;
};

} // namespace Testing
} // namespace Infrastructure

#endif // TEAMEXECUTIONMONITOR_H
