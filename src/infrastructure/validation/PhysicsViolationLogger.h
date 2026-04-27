#ifndef PHYSICSVIOLATIONLOGGER_H
#define PHYSICSVIOLATIONLOGGER_H

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDateTime>
#include <QMutex>
#include "PhysicsValidator.h"
#include "../../domain/TelemetrySnapshot.h"

namespace Infrastructure {
namespace Validation {

/**
 * @brief Logger for physics validation violations
 *
 * Outputs violations in JSON Lines format for offline analysis.
 * Thread-safe for concurrent logging.
 */
class PhysicsViolationLogger {
public:
    explicit PhysicsViolationLogger(const QString& logDirectory = "logs");
    ~PhysicsViolationLogger();

    /**
     * @brief Start a new log session
     * Creates a new log file with timestamp
     */
    bool startSession(const QString& testSerialNumber = QString());

    /**
     * @brief Log a validation violation
     */
    void logViolation(const PhysicsValidator::ValidationResult& result,
                      const Domain::TelemetrySnapshot& snapshot);

    /**
     * @brief Close current log session
     */
    void closeSession();

    /**
     * @brief Get current log file path
     */
    QString currentLogPath() const { return m_currentLogPath; }

    /**
     * @brief Check if logging is active
     */
    bool isActive() const { return m_logFile != nullptr && m_logFile->isOpen(); }

private:
    QString m_logDirectory;
    QString m_currentLogPath;
    QFile* m_logFile;
    QTextStream* m_logStream;
    QMutex m_mutex;

    QString generateLogFileName(const QString& testSerialNumber);
    QJsonObject createViolationJson(const PhysicsValidator::ValidationResult& result,
                                     const Domain::TelemetrySnapshot& snapshot);
};

} // namespace Validation
} // namespace Infrastructure

#endif // PHYSICSVIOLATIONLOGGER_H
