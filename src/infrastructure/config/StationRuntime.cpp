#include "StationRuntime.h"
#include <QDebug>

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
    if (!initializeBus("AQMD", m_aqmdBusConfig, m_aqmdBus, m_motor != nullptr)) {
        m_lastError = QString("Failed to open AQMD bus: %1").arg(m_aqmdBus->lastError());
        return false;
    }
    if (!initializeBus("DYN200", m_dyn200BusConfig, m_dyn200Bus, m_torque != nullptr)) {
        return false;
    }
    if (!initializeBus("encoder", m_encoderBusConfig, m_encoderBus, m_encoder != nullptr)) {
        return false;
    }
    if (!initializeBus("brake", m_brakeBusConfig, m_brakeBus, m_brake != nullptr)) {
        return false;
    }

    if (m_motor && !m_motor->initialize()) {
        m_lastError = QString("Failed to initialize motor: %1").arg(m_motor->lastError());
        return false;
    }

    if (m_torque && !m_torque->initialize()) {
        m_lastError = QString("Failed to initialize torque sensor: %1").arg(m_torque->lastError());
        return false;
    }

    if (m_encoder && !m_encoder->initialize()) {
        m_lastError = QString("Failed to initialize encoder: %1").arg(m_encoder->lastError());
        return false;
    }

    if (m_brake && !m_brake->initialize()) {
        m_lastError = QString("Failed to initialize brake: %1").arg(m_brake->lastError());
        return false;
    }

    if (m_acquisitionScheduler && !m_acquisitionScheduler->start()) {
        qWarning() << "Failed to start acquisition scheduler (non-fatal, will use synchronous fallback)";
    }

    m_initialized = true;
    qDebug() << "Station runtime initialized successfully";
    return true;
}

bool StationRuntime::initializeBus(const QString& displayName,
                                   const BusConfig& config,
                                   const std::unique_ptr<Bus::ModbusRtuBusController>& bus,
                                   bool enabled) {
    if (!enabled) {
        qDebug() << displayName << "device disabled in station config, skipping bus initialization";
        return true;
    }
    if (bus && !bus->open(config.portName, config.baudRate, config.timeoutMs, config.parity, config.stopBits)) {
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
