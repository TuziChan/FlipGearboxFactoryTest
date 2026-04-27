#include "JsonReportWriter.h"
#include <QFile>
#include <QJsonDocument>
#include <QDir>
#include <QDebug>

namespace Infrastructure {
namespace Reporting {

JsonReportWriter::JsonReportWriter()
    : m_lastError()
{
}

bool JsonReportWriter::writeReport(const Domain::TestResults& results,
                                    const Acquisition::TelemetryBuffer& telemetry,
                                    const QString& filePath) {
    QFileInfo fileInfo(filePath);
    QDir dir = fileInfo.dir();
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            m_lastError = QString("Failed to create directory: %1").arg(dir.path());
            return false;
        }
    }

    QJsonObject reportJson;
    reportJson["report_version"] = "1.0";
    reportJson["generated_at"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    reportJson["results"] = serializeResults(results);
    reportJson["telemetry"] = serializeTelemetry(telemetry);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = QString("Failed to open file for writing: %1").arg(filePath);
        return false;
    }

    QJsonDocument doc(reportJson);
    qint64 written = file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    if (written == -1) {
        m_lastError = "Failed to write JSON data to file";
        return false;
    }

    return true;
}

QJsonObject JsonReportWriter::serializeResults(const Domain::TestResults& results) {
    QJsonObject obj;

    obj["serial_number"] = results.serialNumber;
    obj["recipe_name"] = results.recipeName;
    obj["station_name"] = results.stationName;
    obj["start_time"] = results.startTime.toString(Qt::ISODate);
    obj["end_time"] = results.endTime.toString(Qt::ISODate);
    obj["homing_completed"] = results.homingCompleted;
    obj["final_encoder_zero_deg"] = results.finalEncoderZeroDeg;
    obj["overall_passed"] = results.overallPassed;

    QJsonObject idleObj;
    idleObj["forward_current_avg"] = results.idleForward.currentAvg;
    idleObj["forward_current_max"] = results.idleForward.currentMax;
    idleObj["forward_speed_avg"] = results.idleForward.speedAvg;
    idleObj["forward_speed_max"] = results.idleForward.speedMax;
    idleObj["reverse_current_avg"] = results.idleReverse.currentAvg;
    idleObj["reverse_current_max"] = results.idleReverse.currentMax;
    idleObj["reverse_speed_avg"] = results.idleReverse.speedAvg;
    idleObj["reverse_speed_max"] = results.idleReverse.speedMax;
    obj["idle_run"] = idleObj;

    QJsonArray angleArray;
    for (const auto& angle : results.angleResults) {
        QJsonObject angleObj;
        angleObj["position"] = angle.positionName;
        angleObj["target_deg"] = angle.targetAngleDeg;
        angleObj["measured_deg"] = angle.measuredAngleDeg;
        angleObj["deviation_deg"] = angle.deviationDeg;
        angleObj["tolerance_deg"] = angle.toleranceDeg;
        angleObj["passed"] = angle.passed;
        angleArray.append(angleObj);
    }
    obj["angle_results"] = angleArray;

    QJsonObject loadObj;
    loadObj["forward_lock_current"] = results.loadForward.lockCurrentA;
    loadObj["forward_lock_torque"] = results.loadForward.lockTorqueNm;
    loadObj["forward_lock_achieved"] = results.loadForward.lockAchieved;
    loadObj["reverse_lock_current"] = results.loadReverse.lockCurrentA;
    loadObj["reverse_lock_torque"] = results.loadReverse.lockTorqueNm;
    loadObj["reverse_lock_achieved"] = results.loadReverse.lockAchieved;
    obj["load_test"] = loadObj;

    return obj;
}

QJsonObject JsonReportWriter::serializeTelemetry(const Acquisition::TelemetryBuffer& telemetry) {
    QJsonObject obj;

    QJsonObject motorObj;
    motorObj["current_a"] = telemetry.motor.currentA.load();
    motorObj["ai1_level"] = telemetry.motor.ai1Level.load();
    motorObj["valid"] = telemetry.motor.valid.load();
    motorObj["error_count"] = telemetry.motor.errorCount.load();
    motorObj["success_count"] = telemetry.motor.successCount.load();
    obj["motor"] = motorObj;

    QJsonObject torqueObj;
    torqueObj["torque_nm"] = telemetry.torque.torqueNm.load();
    torqueObj["speed_rpm"] = telemetry.torque.speedRpm.load();
    torqueObj["power_w"] = telemetry.torque.powerW.load();
    torqueObj["valid"] = telemetry.torque.valid.load();
    torqueObj["error_count"] = telemetry.torque.errorCount.load();
    torqueObj["success_count"] = telemetry.torque.successCount.load();
    obj["torque"] = torqueObj;

    QJsonObject encoderObj;
    encoderObj["angle_deg"] = telemetry.encoder.angleDeg.load();
    encoderObj["total_angle_deg"] = telemetry.encoder.totalAngleDeg.load();
    encoderObj["velocity_rpm"] = telemetry.encoder.velocityRpm.load();
    encoderObj["valid"] = telemetry.encoder.valid.load();
    encoderObj["error_count"] = telemetry.encoder.errorCount.load();
    encoderObj["success_count"] = telemetry.encoder.successCount.load();
    obj["encoder"] = encoderObj;

    QJsonObject brakeObj;
    brakeObj["current_a"] = telemetry.brake.currentA.load();
    brakeObj["voltage_v"] = telemetry.brake.voltageV.load();
    brakeObj["power_w"] = telemetry.brake.powerW.load();
    brakeObj["valid"] = telemetry.brake.valid.load();
    brakeObj["error_count"] = telemetry.brake.errorCount.load();
    brakeObj["success_count"] = telemetry.brake.successCount.load();
    obj["brake"] = brakeObj;

    return obj;
}

} // namespace Reporting
} // namespace Infrastructure
