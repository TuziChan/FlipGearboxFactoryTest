#include "PhysicsViolationLogger.h"
#include <QDir>
#include <QJsonArray>

namespace Infrastructure {
namespace Validation {

PhysicsViolationLogger::PhysicsViolationLogger(const QString& logDirectory)
    : m_logDirectory(logDirectory)
    , m_logFile(nullptr)
    , m_logStream(nullptr)
{
    // Ensure log directory exists
    QDir dir;
    if (!dir.exists(m_logDirectory)) {
        dir.mkpath(m_logDirectory);
    }
}

PhysicsViolationLogger::~PhysicsViolationLogger()
{
    closeSession();
}

bool PhysicsViolationLogger::startSession(const QString& testSerialNumber)
{
    QMutexLocker locker(&m_mutex);

    // Close existing session if any
    if (m_logFile && m_logFile->isOpen()) {
        closeSession();
    }

    // Generate log file name
    m_currentLogPath = generateLogFileName(testSerialNumber);

    // Open log file
    m_logFile = new QFile(m_currentLogPath);
    if (!m_logFile->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
        delete m_logFile;
        m_logFile = nullptr;
        return false;
    }

    m_logStream = new QTextStream(m_logFile);
    m_logStream->setEncoding(QStringConverter::Utf8);

    return true;
}

void PhysicsViolationLogger::logViolation(
    const PhysicsValidator::ValidationResult& result,
    const Domain::TelemetrySnapshot& snapshot)
{
    QMutexLocker locker(&m_mutex);

    if (!m_logStream) {
        return;
    }

    // Create JSON object
    QJsonObject json = createViolationJson(result, snapshot);

    // Write as JSON Lines (one JSON object per line)
    QJsonDocument doc(json);
    *m_logStream << doc.toJson(QJsonDocument::Compact) << "\n";
    m_logStream->flush();
}

void PhysicsViolationLogger::closeSession()
{
    QMutexLocker locker(&m_mutex);

    if (m_logStream) {
        delete m_logStream;
        m_logStream = nullptr;
    }

    if (m_logFile) {
        if (m_logFile->isOpen()) {
            m_logFile->close();
        }
        delete m_logFile;
        m_logFile = nullptr;
    }

    m_currentLogPath.clear();
}

QString PhysicsViolationLogger::generateLogFileName(const QString& testSerialNumber)
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString fileName;

    if (!testSerialNumber.isEmpty()) {
        fileName = QString("physics_violations_%1_%2.jsonl")
            .arg(testSerialNumber)
            .arg(timestamp);
    } else {
        fileName = QString("physics_violations_%1.jsonl").arg(timestamp);
    }

    return QDir(m_logDirectory).filePath(fileName);
}

QJsonObject PhysicsViolationLogger::createViolationJson(
    const PhysicsValidator::ValidationResult& result,
    const Domain::TelemetrySnapshot& snapshot)
{
    QJsonObject json;

    // Timestamp
    json["timestamp"] = snapshot.timestamp.toString(Qt::ISODateWithMs);

    // Violation details
    json["rule"] = result.ruleName;
    json["passed"] = result.passed;
    json["message"] = result.message;
    json["actual_value"] = result.actualValue;
    json["expected_value"] = result.expectedValue;
    json["threshold"] = result.threshold;
    json["error_percent"] = result.errorPercent;

    // Telemetry snapshot (for context)
    QJsonObject telemetry;
    telemetry["motor_current_a"] = snapshot.motorCurrentA;
    telemetry["dyn_speed_rpm"] = snapshot.dynSpeedRpm;
    telemetry["dyn_torque_nm"] = snapshot.dynTorqueNm;
    telemetry["dyn_power_w"] = snapshot.dynPowerW;
    telemetry["encoder_angle_deg"] = snapshot.encoderAngleDeg;
    telemetry["encoder_velocity_rpm"] = snapshot.encoderVelocityRpm;
    telemetry["brake_current_a"] = snapshot.brakeCurrentA;
    telemetry["brake_voltage_v"] = snapshot.brakeVoltageV;
    telemetry["brake_power_w"] = snapshot.brakePowerW;

    json["telemetry"] = telemetry;

    return json;
}

} // namespace Validation
} // namespace Infrastructure
