#include "DiagnosticsViewModel.h"
#include "../infrastructure/devices/IMotorDriveDevice.h"
#include <QDateTime>
#include <QRegularExpression>

namespace ViewModels {

namespace {

QString humanizeError(QString message) {
    // Keep this conservative: only translate common transport errors.
    message.replace(QStringLiteral("Serial port is not open"), QStringLiteral("串口未打开"));
    message.replace(QStringLiteral("Bus controller is not open"), QStringLiteral("总线未打开"));

    message.replace(QRegularExpression(QStringLiteral(R"(Timeout: no response received within (\d+) ms)")),
                    QStringLiteral("超时：在 \\1 ms 内未收到响应"));
    message.replace(QRegularExpression(QStringLiteral(R"(Timeout: incomplete response \(received (\d+) bytes, expected at least (\d+)\))")),
                    QStringLiteral("超时：响应不完整（收到 \\1 字节，至少需要 \\2 字节）"));
    message.replace(QRegularExpression(QStringLiteral(R"(Failed to open port ([^:]+): )")),
                    QStringLiteral("打开串口 \\1 失败："));

    return message;
}

QString busOfflineReason(Infrastructure::Bus::IBusController* bus) {
    if (!bus) {
        return QStringLiteral("总线未配置");
    }
    if (bus->isOpen()) {
        return QString();
    }
    const QString raw = bus->lastError();
    if (raw.trimmed().isEmpty()) {
        return QStringLiteral("串口未打开");
    }
    return raw;
}

} // namespace

DiagnosticsViewModel::DiagnosticsViewModel(Infrastructure::Config::StationRuntime* runtime, QObject* parent)
    : QObject(parent)
    , m_runtime(runtime)
    , m_deviceStatuses()
    , m_communicationLogs()
    , m_statusMessage()
    , m_nextRefreshIndex(0)
    , m_motorTelemetry()
    , m_torqueTelemetry()
    , m_encoderTelemetry()
    , m_brakeTelemetry()
{
    initializeStatuses();
    refresh();
}

bool DiagnosticsViewModel::runtimeInitialized() const {
    return m_runtime && m_runtime->isInitialized();
}

void DiagnosticsViewModel::refresh() {
    updateDeviceStatus(0);
    updateDeviceStatus(1);
    updateDeviceStatus(2);
    updateDeviceStatus(3);
    emit deviceStatusesChanged();
    if (runtimeInitialized()) {
        setStatusMessage(QStringLiteral("已刷新真实设备状态"));
        return;
    }

    const QString runtimeError = m_runtime ? m_runtime->lastError() : QString();
    if (!runtimeError.trimmed().isEmpty()) {
        setStatusMessage(humanizeError(runtimeError));
        return;
    }

    setStatusMessage(QStringLiteral("runtime 未完整初始化，当前结果可能包含离线设备"));
}

void DiagnosticsViewModel::refreshIncremental() {
    updateDeviceStatus(m_nextRefreshIndex);
    m_nextRefreshIndex = (m_nextRefreshIndex + 1) % 4;
    emit deviceStatusesChanged();
}

void DiagnosticsViewModel::setMotorForward(double dutyCyclePercent) {
    if (!m_runtime || !m_runtime->motor()) {
        appendLog("发送", "AQMD", "电机未启用，无法正转", false);
        return;
    }
    const bool ok = m_runtime->motor()->setMotor(Infrastructure::Devices::IMotorDriveDevice::Direction::Forward, dutyCyclePercent);
    appendLog("发送", "AQMD", ok ? QString("设置正转 %1%").arg(dutyCyclePercent, 0, 'f', 1)
                                  : QString("设置正转失败: %1").arg(m_runtime->motor()->lastError()), ok);
    refresh();
}

void DiagnosticsViewModel::setMotorReverse(double dutyCyclePercent) {
    if (!m_runtime || !m_runtime->motor()) {
        appendLog("发送", "AQMD", "电机未启用，无法反转", false);
        return;
    }
    const bool ok = m_runtime->motor()->setMotor(Infrastructure::Devices::IMotorDriveDevice::Direction::Reverse, dutyCyclePercent);
    appendLog("发送", "AQMD", ok ? QString("设置反转 %1%").arg(dutyCyclePercent, 0, 'f', 1)
                                  : QString("设置反转失败: %1").arg(m_runtime->motor()->lastError()), ok);
    refresh();
}

void DiagnosticsViewModel::stopMotor() {
    if (!m_runtime || !m_runtime->motor()) {
        appendLog("发送", "AQMD", "电机未启用，无法停止", false);
        return;
    }
    const bool ok = m_runtime->motor()->brake();
    appendLog("发送", "AQMD", ok ? QStringLiteral("停止电机") : QString("停止电机失败: %1").arg(m_runtime->motor()->lastError()), ok);
    refresh();
}

void DiagnosticsViewModel::setBrakeOutput(bool enabled) {
    if (!m_runtime || !m_runtime->brake()) {
        appendLog("发送", "制动电源", "制动电源未启用，无法切换输出", false);
        return;
    }
    const bool ok = m_runtime->brake()->setOutputEnable(m_runtime->brakeChannel(), enabled);
    appendLog("发送", "制动电源", ok ? (enabled ? QStringLiteral("使能输出") : QStringLiteral("禁用输出"))
                                     : QString("切换输出失败: %1").arg(m_runtime->brake()->lastError()), ok);
    refresh();
}

void DiagnosticsViewModel::setBrakeCurrent(double currentA) {
    if (!m_runtime || !m_runtime->brake()) {
        appendLog("发送", "制动电源", "制动电源未启用，无法设置电流", false);
        return;
    }
    const bool ok = m_runtime->brake()->setCurrent(m_runtime->brakeChannel(), currentA);
    appendLog("发送", "制动电源", ok ? QString("设置电流 %1 A").arg(currentA, 0, 'f', 2)
                                     : QString("设置电流失败: %1").arg(m_runtime->brake()->lastError()), ok);
    refresh();
}

void DiagnosticsViewModel::setBrakeVoltage(double voltageV) {
    if (!m_runtime || !m_runtime->brake()) {
        appendLog("发送", "制动电源", "制动电源未启用，无法设置电压", false);
        return;
    }
    const bool ok = m_runtime->brake()->setVoltage(m_runtime->brakeChannel(), voltageV);
    appendLog("发送", "制动电源", ok ? QString("设置电压 %1 V").arg(voltageV, 0, 'f', 2)
                                     : QString("设置电压失败: %1").arg(m_runtime->brake()->lastError()), ok);
    refresh();
}

void DiagnosticsViewModel::setBrakeMode(const QString& mode) {
    if (!m_runtime || !m_runtime->brake()) {
        appendLog("发送", "制动电源", "制动电源未启用，无法切换模式", false);
        return;
    }
    const bool ok = m_runtime->brake()->setBrakeMode(m_runtime->brakeChannel(), mode);
    appendLog("发送", "制动电源", ok ? QString("切换为 %1 模式").arg(mode)
                                     : QString("切换模式失败: %1").arg(m_runtime->brake()->lastError()), ok);
    refresh();
}

void DiagnosticsViewModel::setEncoderZero() {
    if (!m_runtime || !m_runtime->encoder()) {
        appendLog("发送", "编码器", "编码器未启用，无法置零", false);
        return;
    }
    const bool ok = m_runtime->encoder()->setZeroPoint();
    appendLog("发送", "编码器", ok ? QStringLiteral("设置当前点为零位")
                                   : QString("置零失败: %1").arg(m_runtime->encoder()->lastError()), ok);
    refresh();
}

void DiagnosticsViewModel::clearLog() {
    m_communicationLogs.clear();
    emit communicationLogsChanged();
    setStatusMessage(QStringLiteral("通信日志已清空"));
}

void DiagnosticsViewModel::initializeStatuses() {
    m_deviceStatuses = {
        buildOfflineStatus("AQMD 电机驱动器", "等待刷新"),
        buildOfflineStatus("DYN200 扭矩传感器", "等待刷新"),
        buildOfflineStatus("单圈绝对值编码器", "等待刷新"),
        buildOfflineStatus("制动电源", "等待刷新")
    };
}

void DiagnosticsViewModel::updateDeviceStatus(int index) {
    if (index < 0 || index >= 4) {
        return;
    }

    QVariantMap status;
    switch (index) {
        case 0:
            status = buildMotorStatus();
            break;
        case 1:
            status = buildTorqueStatus();
            break;
        case 2:
            status = buildEncoderStatus();
            break;
        case 3:
            status = buildBrakeStatus();
            break;
        default:
            return;
    }

    m_deviceStatuses[index] = status;
}

QVariantMap DiagnosticsViewModel::buildMotorStatus() {
    if (!m_runtime || !m_runtime->motor()) {
        QVariantMap newMotorTelemetry{
            {"currentA", 0.0},
            {"ai1Level", false},
            {"online", false}
        };
        if (m_motorTelemetry != newMotorTelemetry) {
            m_motorTelemetry = newMotorTelemetry;
            emit motorTelemetryChanged();
        }
        return buildOfflineStatus("AQMD 电机驱动器", "设备未启用");
    }

    const QString motorBusReason = busOfflineReason(m_runtime->aqmdBus());
    if (!motorBusReason.isEmpty()) {
        QVariantMap newMotorTelemetry{
            {"currentA", 0.0},
            {"ai1Level", false},
            {"online", false}
        };
        if (m_motorTelemetry != newMotorTelemetry) {
            m_motorTelemetry = newMotorTelemetry;
            emit motorTelemetryChanged();
        }
        return buildOfflineStatus("AQMD 电机驱动器", motorBusReason);
    }

    double currentA = 0.0;
    bool ai1Level = false;
    const bool currentOk = m_runtime->motor()->readCurrent(currentA);
    const bool ai1Ok = m_runtime->motor()->readAI1Level(ai1Level);
    const bool online = currentOk && ai1Ok;
    
    QVariantMap newMotorTelemetry{
        {"currentA", currentA},
        {"ai1Level", ai1Level},
        {"online", online}
    };
    if (m_motorTelemetry != newMotorTelemetry) {
        m_motorTelemetry = newMotorTelemetry;
        emit motorTelemetryChanged();
    }
    
    return {
        {"name", "AQMD 电机驱动器"},
        {"status", online ? "online" : "offline"},
        {"lastUpdate", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")},
        {"summary", online ? QString("电流 %1 A / AI1 %2").arg(currentA, 0, 'f', 2).arg(ai1Level ? "高" : "低")
                            : QString("通信失败: %1").arg(humanizeError(m_runtime->motor()->lastError()))},
        {"errorCount", online ? 0 : 1}
    };
}

QVariantMap DiagnosticsViewModel::buildTorqueStatus() {
    if (!m_runtime || !m_runtime->torque()) {
        QVariantMap newTorqueTelemetry{
            {"torqueNm", 0.0},
            {"speedRpm", 0.0},
            {"powerW", 0.0},
            {"online", false}
        };
        if (m_torqueTelemetry != newTorqueTelemetry) {
            m_torqueTelemetry = newTorqueTelemetry;
            emit torqueTelemetryChanged();
        }
        return buildOfflineStatus("DYN200 扭矩传感器", "设备未启用");
    }

    const QString torqueBusReason = busOfflineReason(m_runtime->dyn200Bus());
    if (!torqueBusReason.isEmpty()) {
        QVariantMap newTorqueTelemetry{
            {"torqueNm", 0.0},
            {"speedRpm", 0.0},
            {"powerW", 0.0},
            {"online", false}
        };
        if (m_torqueTelemetry != newTorqueTelemetry) {
            m_torqueTelemetry = newTorqueTelemetry;
            emit torqueTelemetryChanged();
        }
        return buildOfflineStatus("DYN200 扭矩传感器", torqueBusReason);
    }

    double torqueNm = 0.0;
    double speedRpm = 0.0;
    double powerW = 0.0;
    const bool ok = m_runtime->torque()->readAll(torqueNm, speedRpm, powerW);
    
    QVariantMap newTorqueTelemetry{
        {"torqueNm", torqueNm},
        {"speedRpm", speedRpm},
        {"powerW", powerW},
        {"online", ok}
    };
    if (m_torqueTelemetry != newTorqueTelemetry) {
        m_torqueTelemetry = newTorqueTelemetry;
        emit torqueTelemetryChanged();
    }
    
    return {
        {"name", "DYN200 扭矩传感器"},
        {"status", ok ? "online" : "offline"},
        {"lastUpdate", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")},
        {"summary", ok ? QString("扭矩 %1 N·m / 转速 %2 RPM / 功率 %3 W")
                               .arg(torqueNm, 0, 'f', 2)
                               .arg(speedRpm, 0, 'f', 1)
                               .arg(powerW, 0, 'f', 1)
                         : QString("通信失败: %1").arg(humanizeError(m_runtime->torque()->lastError()))},
        {"errorCount", ok ? 0 : 1}
    };
}

QVariantMap DiagnosticsViewModel::buildEncoderStatus() {
    if (!m_runtime || !m_runtime->encoder()) {
        QVariantMap newEncoderTelemetry{
            {"angleDeg", 0.0},
            {"online", false}
        };
        if (m_encoderTelemetry != newEncoderTelemetry) {
            m_encoderTelemetry = newEncoderTelemetry;
            emit encoderTelemetryChanged();
        }
        return buildOfflineStatus("单圈绝对值编码器", "设备未启用");
    }

    const QString encoderBusReason = busOfflineReason(m_runtime->encoderBus());
    if (!encoderBusReason.isEmpty()) {
        QVariantMap newEncoderTelemetry{
            {"angleDeg", 0.0},
            {"online", false}
        };
        if (m_encoderTelemetry != newEncoderTelemetry) {
            m_encoderTelemetry = newEncoderTelemetry;
            emit encoderTelemetryChanged();
        }
        return buildOfflineStatus("单圈绝对值编码器", encoderBusReason);
    }

    double angleDeg = 0.0;
    const bool ok = m_runtime->encoder()->readAngle(angleDeg);
    
    QVariantMap newEncoderTelemetry{
        {"angleDeg", angleDeg},
        {"online", ok}
    };
    if (m_encoderTelemetry != newEncoderTelemetry) {
        m_encoderTelemetry = newEncoderTelemetry;
        emit encoderTelemetryChanged();
    }
    
    return {
        {"name", "单圈绝对值编码器"},
        {"status", ok ? "online" : "offline"},
        {"lastUpdate", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")},
        {"summary", ok ? QString("角度 %1°").arg(angleDeg, 0, 'f', 2)
                        : QString("通信失败: %1").arg(humanizeError(m_runtime->encoder()->lastError()))},
        {"errorCount", ok ? 0 : 1}
    };
}

QVariantMap DiagnosticsViewModel::buildBrakeStatus() {
    if (!m_runtime || !m_runtime->brake()) {
        QVariantMap newBrakeTelemetry{
            {"currentA", 0.0},
            {"voltageV", 0.0},
            {"powerW", 0.0},
            {"mode", "CC"},
            {"channel", 0},
            {"online", false}
        };
        if (m_brakeTelemetry != newBrakeTelemetry) {
            m_brakeTelemetry = newBrakeTelemetry;
            emit brakeTelemetryChanged();
        }
        return buildOfflineStatus("制动电源", "设备未启用");
    }

    const QString brakeBusReason = busOfflineReason(m_runtime->brakeBus());
    if (!brakeBusReason.isEmpty()) {
        QVariantMap newBrakeTelemetry{
            {"currentA", 0.0},
            {"voltageV", 0.0},
            {"powerW", 0.0},
            {"mode", "CC"},
            {"channel", 0},
            {"online", false}
        };
        if (m_brakeTelemetry != newBrakeTelemetry) {
            m_brakeTelemetry = newBrakeTelemetry;
            emit brakeTelemetryChanged();
        }
        return buildOfflineStatus("制动电源", brakeBusReason);
    }

    double currentA = 0.0;
    double voltageV = 0.0;
    double powerW = 0.0;
    int mode = 0;
    const bool currentOk = m_runtime->brake()->readCurrent(m_runtime->brakeChannel(), currentA);
    const bool voltageOk = m_runtime->brake()->readVoltage(m_runtime->brakeChannel(), voltageV);
    m_runtime->brake()->readPower(m_runtime->brakeChannel(), powerW);
    m_runtime->brake()->readMode(m_runtime->brakeChannel(), mode);
    const bool online = currentOk && voltageOk;
    QString modeStr = (mode == 1) ? "CV" : "CC";
    
    QVariantMap newBrakeTelemetry{
        {"currentA", currentA},
        {"voltageV", voltageV},
        {"powerW", powerW},
        {"mode", modeStr},
        {"channel", m_runtime->brakeChannel()},
        {"online", online}
    };
    if (m_brakeTelemetry != newBrakeTelemetry) {
        m_brakeTelemetry = newBrakeTelemetry;
        emit brakeTelemetryChanged();
    }
    
    return {
        {"name", "制动电源"},
        {"status", online ? "online" : "offline"},
        {"lastUpdate", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")},
        {"summary", online ? QString("通道 %1 / %2 / 电流 %3 A / 电压 %4 V / 功率 %5 W")
                                    .arg(m_runtime->brakeChannel())
                                    .arg(modeStr)
                                    .arg(currentA, 0, 'f', 2)
                                    .arg(voltageV, 0, 'f', 2)
                                    .arg(powerW, 0, 'f', 1)
                           : QString("通信失败: %1").arg(humanizeError(m_runtime->brake()->lastError()))},
        {"errorCount", online ? 0 : 1}
    };
}

QVariantMap DiagnosticsViewModel::buildOfflineStatus(const QString& name, const QString& reason) const {
    return {
        {"name", name},
        {"status", "offline"},
        {"lastUpdate", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")},
        {"summary", humanizeError(reason)},
        {"errorCount", 1}
    };
}

void DiagnosticsViewModel::appendLog(const QString& direction, const QString& device, const QString& message, bool success) {
    QVariantMap entry{
        {"time", QDateTime::currentDateTime().toString("hh:mm:ss")},
        {"direction", direction},
        {"device", device},
        {"message", message},
        {"success", success}
    };
    m_communicationLogs.prepend(entry);
    while (m_communicationLogs.size() > 100) {
        m_communicationLogs.removeLast();
    }
    emit communicationLogsChanged();
    setStatusMessage(message);
}

void DiagnosticsViewModel::setStatusMessage(const QString& message) {
    if (m_statusMessage != message) {
        m_statusMessage = message;
        emit statusMessageChanged();
    }
}

} // namespace ViewModels
