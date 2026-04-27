#include "LogManager.h"
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <iostream>

namespace Infrastructure {
namespace Logging {

LogManager& LogManager::instance() {
    static LogManager instance;
    return instance;
}

LogManager::LogManager()
    : QObject(nullptr)
    , m_logDir("logs")
    , m_maxDays(30)
    , m_logFile(nullptr)
    , m_logStream(nullptr)
    , m_currentDate()
    , m_initialized(false)
    , m_flushTimer(new QTimer(this))
{
    // Flush logs every 1 second to balance performance and data safety
    connect(m_flushTimer, &QTimer::timeout, this, &LogManager::flushLogs);
}

LogManager::~LogManager() {
    shutdown();
}

void LogManager::initialize(const QString& logDir, int maxDays) {
    QMutexLocker locker(&m_mutex);

    if (m_initialized) {
        return;
    }

    m_logDir = logDir;
    m_maxDays = maxDays;

    QDir dir;
    if (!dir.exists(m_logDir)) {
        dir.mkpath(m_logDir);
    }

    rotateLogFile();
    cleanOldLogs();

    qInstallMessageHandler(LogManager::messageHandler);
    m_flushTimer->start(1000); // Flush every 1 second
    m_initialized = true;

    qInfo() << "Log system initialized. Log directory:" << QDir(m_logDir).absolutePath();
}

void LogManager::shutdown() {
    QMutexLocker locker(&m_mutex);

    if (!m_initialized) {
        return;
    }

    m_flushTimer->stop();
    qInstallMessageHandler(nullptr);

    if (m_logStream) {
        m_logStream->flush();
        delete m_logStream;
        m_logStream = nullptr;
    }

    if (m_logFile) {
        m_logFile->close();
        delete m_logFile;
        m_logFile = nullptr;
    }

    m_initialized = false;
}

void LogManager::flushLogs() {
    QMutexLocker locker(&m_mutex);
    if (m_logStream) {
        m_logStream->flush();
    }
}

void LogManager::rotateLogFile() {
    QString currentDate = QDate::currentDate().toString("yyyy-MM-dd");

    if (currentDate == m_currentDate && m_logFile && m_logFile->isOpen()) {
        return;
    }

    if (m_logStream) {
        m_logStream->flush();
        delete m_logStream;
        m_logStream = nullptr;
    }

    if (m_logFile) {
        m_logFile->close();
        delete m_logFile;
        m_logFile = nullptr;
    }

    m_currentDate = currentDate;
    QString logFileName = getCurrentLogFileName();

    m_logFile = new QFile(logFileName);
    if (m_logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        m_logStream = new QTextStream(m_logFile);
        m_logStream->setEncoding(QStringConverter::Utf8);
    }
}

void LogManager::cleanOldLogs() {
    QDir dir(m_logDir);
    QStringList filters;
    filters << "app_*.log";
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files, QDir::Time | QDir::Reversed);

    QDate cutoffDate = QDate::currentDate().addDays(-m_maxDays);

    for (const QFileInfo& fileInfo : files) {
        if (fileInfo.lastModified().date() < cutoffDate) {
            QFile::remove(fileInfo.absoluteFilePath());
        }
    }
}

QString LogManager::getCurrentLogFileName() const {
    return QString("%1/app_%2.log").arg(m_logDir, m_currentDate);
}

void LogManager::messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
    LogManager& manager = instance();
    QMutexLocker locker(&manager.m_mutex);

    if (!manager.m_initialized) {
        return;
    }

    manager.rotateLogFile();

    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
    QString typeStr;

    switch (type) {
    case QtDebugMsg:
        typeStr = "DEBUG";
        break;
    case QtInfoMsg:
        typeStr = "INFO ";
        break;
    case QtWarningMsg:
        typeStr = "WARN ";
        break;
    case QtCriticalMsg:
        typeStr = "ERROR";
        break;
    case QtFatalMsg:
        typeStr = "FATAL";
        break;
    }

    QString contextStr;
    if (context.file && context.line > 0) {
        QFileInfo fileInfo(context.file);
        contextStr = QString(" [%1:%2]").arg(fileInfo.fileName()).arg(context.line);
    }

    QString logLine = QString("[%1] %2%3 - %4\n")
                          .arg(timestamp, typeStr, contextStr, msg);

    if (manager.m_logStream) {
        *manager.m_logStream << logLine;
        // Only flush on critical/fatal messages, not on every log
        if (type >= QtCriticalMsg) {
            manager.m_logStream->flush();
        }
    }

    std::cout << logLine.toStdString();

    if (type == QtFatalMsg) {
        manager.shutdown();
        abort();
    }
}

} // namespace Logging
} // namespace Infrastructure
