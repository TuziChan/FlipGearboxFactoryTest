#include "StationRuntime.h"
#include <QDebug>
#include <QStringList>

namespace Infrastructure {
namespace Config {

StationRuntime::StationRuntime(QObject* parent)
    : QObject(parent)
    , m_lastError()
{
}

StationRuntime::~StationRuntime() {
    shutdown();
}

bool StationRuntime::initialize() {
    qDebug() << "Initializing station runtime...";
    m_initialized = false;
    m_lastError.clear();

    if (m_acquisitionScheduler) {
        m_acquisitionScheduler->stop();
    }

    QStringList errors;

    const bool motorEnabled = (m_motor != nullptr);
    const bool torqueEnabled = (m_torque != nullptr);
    const bool encoderEnabled = (m_encoder != nullptr);
    const bool brakeEnabled = (m_brake != nullptr);

    const bool motorBusOk = initializeBus("AQMD", m_aqmdBusConfig, m_aqmdBus, motorEnabled);
    if (motorEnabled && !motorBusOk) {
        errors << QStringLiteral("AQMD 总线打开失败(%1): %2")
                      .arg(m_aqmdBusConfig.portName,
                           m_aqmdBus ? m_aqmdBus->lastError() : QStringLiteral("bus not configured"));
    }

    const bool torqueBusOk = initializeBus("DYN200", m_dyn200BusConfig, m_dyn200Bus, torqueEnabled);
    if (torqueEnabled && !torqueBusOk) {
        errors << QStringLiteral("DYN200 总线打开失败(%1): %2")
                      .arg(m_dyn200BusConfig.portName,
                           m_dyn200Bus ? m_dyn200Bus->lastError() : QStringLiteral("bus not configured"));
    }

    const bool encoderBusOk = initializeBus("encoder", m_encoderBusConfig, m_encoderBus, encoderEnabled);
    if (encoderEnabled && !encoderBusOk) {
        errors << QStringLiteral("编码器 总线打开失败(%1): %2")
                      .arg(m_encoderBusConfig.portName,
                           m_encoderBus ? m_encoderBus->lastError() : QStringLiteral("bus not configured"));
    }

    const bool brakeBusOk = initializeBus("brake", m_brakeBusConfig, m_brakeBus, brakeEnabled);
    if (brakeEnabled && !brakeBusOk) {
        errors << QStringLiteral("制动电源 总线打开失败(%1): %2")
                      .arg(m_brakeBusConfig.portName,
                           m_brakeBus ? m_brakeBus->lastError() : QStringLiteral("bus not configured"));
    }

    if (m_motor && motorBusOk && !m_motor->initialize()) {
        errors << QStringLiteral("AQMD 初始化失败: %1").arg(m_motor->lastError());
    }

    if (m_torque && torqueBusOk && !m_torque->initialize()) {
        errors << QStringLiteral("DYN200 初始化失败: %1").arg(m_torque->lastError());
    }

    if (m_encoder && encoderBusOk && !m_encoder->initialize()) {
        errors << QStringLiteral("编码器 初始化失败: %1").arg(m_encoder->lastError());
    }

    if (m_brake && brakeBusOk && !m_brake->initialize()) {
        errors << QStringLiteral("制动电源 初始化失败: %1").arg(m_brake->lastError());
    }

    if (!errors.isEmpty()) {
        m_lastError = QStringLiteral("runtime 初始化未完成：%1").arg(errors.join(QStringLiteral("；")));
        qWarning() << m_lastError;
        return false;
    }

    if (m_acquisitionScheduler && !m_acquisitionScheduler->start()) {
        qWarning() << "Failed to start acquisition scheduler (non-fatal, will use synchronous fallback)";
    }

    m_initialized = true;
    m_lastError.clear();
    qDebug() << "Station runtime initialized successfully";
    return true;
}

bool StationRuntime::initializeBus(const QString& displayName,
                                   const BusConfig& config,
                                   const std::unique_ptr<Bus::IBusController>& bus,
                                   bool enabled) {
    if (!enabled) {
        qDebug() << displayName << "device disabled in station config, skipping bus initialization";
        return true;
    }
    if (!bus) {
        m_lastError = QString("Bus controller missing for %1").arg(displayName);
        return false;
    }
    if (!bus->open(config.portName, config.baudRate, config.timeoutMs, config.parity, config.stopBits)) {
        m_lastError = QString("Failed to open %1 bus: %2").arg(displayName, bus->lastError());
        return false;
    }
    return true;
}

void StationRuntime::shutdown() {
    qDebug() << "Shutting down station runtime...";
    m_initialized = false;

    if (m_acquisitionScheduler) {
        m_acquisitionScheduler->stop();
    }

    if (m_testEngine) {
        m_testEngine->reset();
    }

    if (m_aqmdBus) m_aqmdBus->close();
    if (m_dyn200Bus) m_dyn200Bus->close();
    if (m_encoderBus) m_encoderBus->close();
    if (m_brakeBus) m_brakeBus->close();
}

} // namespace Config
} // namespace Infrastructure
