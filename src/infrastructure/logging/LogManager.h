#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QMutex>
#include <QDateTime>

namespace Infrastructure {
namespace Logging {

class LogManager {
public:
    static LogManager& instance();

    void initialize(const QString& logDir = "logs", int maxDays = 30);
    void shutdown();

    static void messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg);

private:
    LogManager();
    ~LogManager();
    LogManager(const LogManager&) = delete;
    LogManager& operator=(const LogManager&) = delete;

    void rotateLogFile();
    void cleanOldLogs();
    QString getCurrentLogFileName() const;

    QString m_logDir;
    int m_maxDays;
    QFile* m_logFile;
    QTextStream* m_logStream;
    QMutex m_mutex;
    QString m_currentDate;
    bool m_initialized;
};

} // namespace Logging
} // namespace Infrastructure

#endif // LOGMANAGER_H
