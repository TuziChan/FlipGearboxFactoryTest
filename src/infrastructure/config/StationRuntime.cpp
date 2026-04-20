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
    qDebug() << "========================================";
    qDebug() << "Initializing station runtime...";
    qDebug() << "========================================";
    
    m_initialized = false;
    m_lastError.clear();

    if (m_acquisitionScheduler) {
        m_acquisitionScheduler->stop();
    }

    QStringList errors;
    QStringList warnings;

    const bool motorEnabled = (m_motor != nullptr);
    const bool torqueEnabled = (m_torque != nullptr);
    const bool encoderEnabled = (m_encoder != nullptr);
    const bool brakeEnabled = (m_brake != nullptr);

    qDebug() << "Device configuration:";
    qDebug() << "  - Motor (AQMD):" << (motorEnabled ? "enabled" : "disabled");
    qDebug() << "  - Torque (DYN200):" << (torqueEnabled ? "enabled" : "disabled");
    qDebug() << "  - Encoder:" << (encoderEnabled ? "enabled" : "disabled");
    qDebug() << "  - Brake:" << (brakeEnabled ? "enabled" : "disabled");

    // Phase 1: Initialize buses
    qDebug() << "Phase 1: Initializing communication buses...";
    
    const bool motorBusOk = initializeBus("AQMD", m_aqmdBusConfig, m_aqmdBus, motorEnabled);
    if (motorEnabled && !motorBusOk) {
        QString error = QStringLiteral("AQMD 总线打开失败(%1): %2")
                      .arg(m_aqmdBusConfig.portName,
                           m_aqmdBus ? m_aqmdBus->lastError() : QStringLiteral("bus not configured"));
        errors << error;
        qCritical() << "  [FAILED]" << error;
    } else if (motorEnabled) {
        qDebug() << "  [OK] AQMD bus opened on" << m_aqmdBusConfig.portName;
    }

    const bool torqueBusOk = initializeBus("DYN200", m_dyn200BusConfig, m_dyn200Bus, torqueEnabled);
    if (torqueEnabled && !torqueBusOk) {
        QString error = QStringLiteral("DYN200 总线打开失败(%1): %2")
                      .arg(m_dyn200BusConfig.portName,
                           m_dyn200Bus ? m_dyn200Bus->lastError() : QStringLiteral("bus not configured"));
        errors << error;
        qCritical() << "  [FAILED]" << error;
    } else if (torqueEnabled) {
        qDebug() << "  [OK] DYN200 bus opened on" << m_dyn200BusConfig.portName;
    }

    const bool encoderBusOk = initializeBus("encoder", m_encoderBusConfig, m_encoderBus, encoderEnabled);
    if (encoderEnabled && !encoderBusOk) {
        QString error = QStringLiteral("编码器 总线打开失败(%1): %2")
                      .arg(m_encoderBusConfig.portName,
                           m_encoderBus ? m_encoderBus->lastError() : QStringLiteral("bus not configured"));
        errors << error;
        qCritical() << "  [FAILED]" << error;
    } else if (encoderEnabled) {
        qDebug() << "  [OK] Encoder bus opened on" << m_encoderBusConfig.portName;
    }

    const bool brakeBusOk = initializeBus("brake", m_brakeBusConfig, m_brakeBus, brakeEnabled);
    if (brakeEnabled && !brakeBusOk) {
        QString error = QStringLiteral("制动电源 总线打开失败(%1): %2")
                      .arg(m_brakeBusConfig.portName,
                           m_brakeBus ? m_brakeBus->lastError() : QStringLiteral("bus not configured"));
        errors << error;
        qCritical() << "  [FAILED]" << error;
    } else if (brakeEnabled) {
        qDebug() << "  [OK] Brake bus opened on" << m_brakeBusConfig.portName;
    }

    // Phase 2: Initialize devices
    qDebug() << "Phase 2: Initializing devices...";
    
    if (m_motor && motorBusOk && !m_motor->initialize()) {
        QString error = QStringLiteral("AQMD 初始化失败: %1").arg(m_motor->lastError());
        errors << error;
        qCritical() << "  [FAILED]" << error;
    } else if (m_motor && motorBusOk) {
        qDebug() << "  [OK] AQMD motor initialized";
    }

    if (m_torque && torqueBusOk && !m_torque->initialize()) {
        QString error = QStringLiteral("DYN200 初始化失败: %1").arg(m_torque->lastError());
        errors << error;
        qCritical() << "  [FAILED]" << error;
    } else if (m_torque && torqueBusOk) {
        qDebug() << "  [OK] DYN200 torque sensor initialized";
    }

    if (m_encoder && encoderBusOk && !m_encoder->initialize()) {
        QString error = QStringLiteral("编码器 初始化失败: %1").arg(m_encoder->lastError());
        errors << error;
        qCritical() << "  [FAILED]" << error;
    } else if (m_encoder && encoderBusOk) {
        qDebug() << "  [OK] Encoder initialized";
    }

    if (m_brake && brakeBusOk && !m_brake->initialize()) {
        QString error = QStringLiteral("制动电源 初始化失败: %1").arg(m_brake->lastError());
        errors << error;
        qCritical() << "  [FAILED]" << error;
    } else if (m_brake && brakeBusOk) {
        qDebug() << "  [OK] Brake power supply initialized";
    }

    // Check for fatal errors
    if (!errors.isEmpty()) {
        m_lastError = QStringLiteral("runtime 初始化未完成：%1").arg(errors.join(QStringLiteral("；")));
        qCritical() << "========================================";
        qCritical() << "INITIALIZATION FAILED";
        qCritical() << "========================================";
        qCritical() << "Errors encountered:" << errors.size();
        for (const QString& error : errors) {
            qCritical() << "  -" << error;
        }
        qCritical() << "========================================";
        qCritical() << "Performing cleanup of partially initialized resources...";
        
        // Cleanup: Close any successfully opened buses
        if (m_aqmdBus && m_aqmdBus->isOpen()) {
            m_aqmdBus->close();
            qDebug() << "  - Closed AQMD bus";
        }
        if (m_dyn200Bus && m_dyn200Bus->isOpen()) {
            m_dyn200Bus->close();
            qDebug() << "  - Closed DYN200 bus";
        }
        if (m_encoderBus && m_encoderBus->isOpen()) {
            m_encoderBus->close();
            qDebug() << "  - Closed encoder bus";
        }
        if (m_brakeBus && m_brakeBus->isOpen()) {
            m_brakeBus->close();
            qDebug() << "  - Closed brake bus";
        }
        
        qCritical() << "Cleanup complete. System is NOT operational.";
        qCritical() << "========================================";
        return false;
    }

    // Phase 3: Start acquisition scheduler (non-fatal)
    qDebug() << "Phase 3: Starting acquisition scheduler...";
    if (m_acquisitionScheduler && !m_acquisitionScheduler->start()) {
        QString warning = "Failed to start acquisition scheduler (non-fatal, will use synchronous fallback)";
        warnings << warning;
        qWarning() << "  [WARNING]" << warning;
    } else if (m_acquisitionScheduler) {
        qDebug() << "  [OK] Acquisition scheduler started";
    }

    m_initialized = true;
    m_lastError.clear();
    
    qDebug() << "========================================";
    qDebug() << "Station runtime initialized successfully";
    if (!warnings.isEmpty()) {
        qDebug() << "Warnings:" << warnings.size();
        for (const QString& warning : warnings) {
            qDebug() << "  -" << warning;
        }
    }
    qDebug() << "========================================";
    
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
