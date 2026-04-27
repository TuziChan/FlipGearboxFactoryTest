#include "SingleTurnEncoderDevice.h"
#include "../bus/ModbusFrame.h"
#include <QDebug>
#include <QThread>

namespace Infrastructure {
namespace Devices {

SingleTurnEncoderDevice::SingleTurnEncoderDevice(Bus::IBusController* busController, 
                                                   uint8_t slaveId,
                                                   uint16_t resolution,
                                                   int communicationMode,
                                                   int autoReportIntervalMs,
                                                   QObject* parent)
    : IEncoderDevice(parent)
    , m_busController(busController)
    , m_slaveId(slaveId)
    , m_resolution(resolution)
    , m_communicationMode(communicationMode)
    , m_autoReportIntervalMs(autoReportIntervalMs)
    , m_lastError()
    , m_proactiveListener(nullptr)
{
}

SingleTurnEncoderDevice::~SingleTurnEncoderDevice() {
    if (m_proactiveListener) {
        m_proactiveListener->stop();
        delete m_proactiveListener;
        m_proactiveListener = nullptr;
    }
}

bool SingleTurnEncoderDevice::initialize() {
    if (!m_busController || !m_busController->isOpen()) {
        m_lastError = "Bus controller is not open";
        emit errorOccurred(m_lastError);
        return false;
    }

    if (m_communicationMode == 0) {
        // Mode 0: Query mode (default)
        // Try reading angle to verify communication
        double angle;
        if (!readAngle(angle)) {
            m_lastError = QString("Failed to verify encoder communication: %1").arg(m_lastError);
            emit errorOccurred(m_lastError);
            return false;
        }

        qDebug() << "Single-turn encoder initialized in query mode, slave ID:" << m_slaveId 
                 << "Resolution:" << m_resolution
                 << "Current angle:" << angle << "deg";
    } else {
        // Modes 2, 3, 4: Auto-report modes
        uint16_t modeValue;
        QString modeName;
        EncoderProactiveListener::ProtocolMode listenerMode;

        switch (m_communicationMode) {
            case 2: // Auto-report single-turn value
                modeValue = 0x01;
                modeName = "auto-report single-turn";
                listenerMode = EncoderProactiveListener::ProtocolMode::SingleTurn;
                break;
            case 3: // Auto-report virtual multi-turn value
                modeValue = 0x04;
                modeName = "auto-report virtual multi-turn";
                listenerMode = EncoderProactiveListener::ProtocolMode::MultiTurn;
                break;
            case 4: // Auto-report angular velocity
                modeValue = 0x05;
                modeName = "auto-report angular velocity";
                listenerMode = EncoderProactiveListener::ProtocolMode::Velocity;
                break;
            default:
                m_lastError = QString("Invalid communication mode: %1").arg(m_communicationMode);
                emit errorOccurred(m_lastError);
                return false;
        }

        // Write auto-report mode to register 0x0006
        if (!writeRegister(REG_AUTO_REPORT_MODE, modeValue)) {
            m_lastError = QString("Failed to set encoder auto-report mode: %1").arg(m_lastError);
            emit errorOccurred(m_lastError);
            return false;
        }

        // Write auto-report interval to register 0x0007
        if (!writeRegister(REG_AUTO_REPORT_INTERVAL, static_cast<uint16_t>(m_autoReportIntervalMs))) {
            m_lastError = QString("Failed to set encoder auto-report interval: %1").arg(m_lastError);
            emit errorOccurred(m_lastError);
            return false;
        }

        // Wait for device to apply the new auto-report mode before starting listener
        QThread::msleep(200);

        // Get underlying serial port for proactive listener
        QSerialPort* serialPort = m_busController->underlyingSerialPort();
        if (!serialPort) {
            m_lastError = "Bus controller does not provide underlying serial port access";
            emit errorOccurred(m_lastError);
            return false;
        }

        // Create and start proactive listener with correct protocol mode
        m_proactiveListener = new EncoderProactiveListener(serialPort, listenerMode, m_resolution, this);
        m_proactiveListener->start();
        
        qDebug() << "Single-turn encoder initialized in" << modeName << "mode, slave ID:" << m_slaveId
                 << "Resolution:" << m_resolution
                 << "Interval:" << m_autoReportIntervalMs << "ms"
                 << "Proactive listener started";
    }

    return true;
}

bool SingleTurnEncoderDevice::readAngle(double& angleDeg) {
    // Only use listener cache in SingleTurn proactive mode (Mode 2)
    if (m_communicationMode == 2 && m_proactiveListener && m_proactiveListener->isValid()) {
        angleDeg = m_proactiveListener->latestAngle();
        return true;
    }

    // Fallback to Modbus polling for Mode 0, 3, 4
    // Query mode (Mode 0) returns 1 register, auto-report modes return 2 registers
    QVector<uint16_t> values;
    uint16_t registerCount = (m_communicationMode == 0) ? 1 : 2;

    if (!readRegisters(REG_ANGLE, registerCount, values) || values.isEmpty()) {
        m_lastError = QString("Failed to read angle: %1").arg(m_lastError);
        return false;
    }

    uint32_t rawCount = 0;
    if (values.size() >= 2) {
        rawCount = (static_cast<uint32_t>(values[0]) << 16) | static_cast<uint32_t>(values[1]);
    } else {
        rawCount = static_cast<uint32_t>(values[0]);
    }

    angleDeg = (static_cast<double>(rawCount) / static_cast<double>(m_resolution)) * 360.0;
    return true;
}

bool SingleTurnEncoderDevice::readVirtualMultiTurn(double& totalAngleDeg) {
    // Only use listener cache in MultiTurn proactive mode (Mode 3)
    if (m_communicationMode == 3 && m_proactiveListener && m_proactiveListener->isValid()) {
        totalAngleDeg = m_proactiveListener->latestMultiTurn();
        return true;
    }

    // Fallback to Modbus polling
    QVector<uint16_t> values;
    if (!readRegisters(REG_VIRTUAL_MULTITURN, 2, values)) {
        m_lastError = QString("Failed to read virtual multi-turn: %1").arg(m_lastError);
        return false;
    }

    uint32_t combined = (static_cast<uint32_t>(values[0]) << 16) | static_cast<uint32_t>(values[1]);
    totalAngleDeg = (static_cast<double>(combined) / static_cast<double>(m_resolution)) * 360.0;
    return true;
}

bool SingleTurnEncoderDevice::readAngularVelocity(double& velocityRpm) {
    // Only use listener cache in Velocity proactive mode (Mode 4)
    if (m_communicationMode == 4 && m_proactiveListener && m_proactiveListener->isValid()) {
        velocityRpm = m_proactiveListener->latestVelocity();
        return true;
    }

    // Fallback to Modbus polling
    QVector<uint16_t> values;
    if (!readRegisters(REG_ANGULAR_VELOCITY, 1, values)) {
        m_lastError = QString("Failed to read angular velocity: %1").arg(m_lastError);
        return false;
    }

    int16_t rawVelocity = static_cast<int16_t>(values[0]);
    velocityRpm = static_cast<double>(rawVelocity);
    return true;
}

bool SingleTurnEncoderDevice::setZeroPoint() {
    if (!writeRegister(REG_SET_ZERO, 1)) {
        m_lastError = QString("Failed to set zero point: %1").arg(m_lastError);
        return false;
    }

    qDebug() << "Encoder zero point set";
    return true;
}

bool SingleTurnEncoderDevice::setAutoReportMode(uint16_t mode, int intervalMs) {
    // Validate mode
    if (mode != 0x00 && mode != 0x01 && mode != 0x04 && mode != 0x05) {
        m_lastError = QString("Invalid auto-report mode: %1").arg(mode);
        emit errorOccurred(m_lastError);
        return false;
    }

    // Write mode register
    if (!writeRegister(REG_AUTO_REPORT_MODE, mode)) {
        m_lastError = QString("Failed to set auto-report mode: %1").arg(m_lastError);
        emit errorOccurred(m_lastError);
        return false;
    }

    // Write interval register
    if (!writeRegister(REG_AUTO_REPORT_INTERVAL, static_cast<uint16_t>(intervalMs))) {
        m_lastError = QString("Failed to set auto-report interval: %1").arg(m_lastError);
        emit errorOccurred(m_lastError);
        return false;
    }

    m_autoReportIntervalMs = intervalMs;

    // Synchronize listener lifecycle
    if (mode == 0x00) {
        // Query mode: stop and delete listener
        if (m_proactiveListener) {
            m_proactiveListener->stop();
            delete m_proactiveListener;  // Use direct delete for immediate cleanup
            m_proactiveListener = nullptr;
        }
        m_communicationMode = 0;
        qDebug() << "Encoder switched to query mode";
    } else {
        // Map register value to communicationMode
        int newCommMode = 0;
        EncoderProactiveListener::ProtocolMode listenerMode;
        if (mode == 0x01) {
            newCommMode = 2;
            listenerMode = EncoderProactiveListener::ProtocolMode::SingleTurn;
        } else if (mode == 0x04) {
            newCommMode = 3;
            listenerMode = EncoderProactiveListener::ProtocolMode::MultiTurn;
        } else if (mode == 0x05) {
            newCommMode = 4;
            listenerMode = EncoderProactiveListener::ProtocolMode::Velocity;
        }

        m_communicationMode = newCommMode;

        // Restart listener with new mode if needed
        if (m_proactiveListener) {
            if (m_proactiveListener->mode() == listenerMode) {
                // Same mode, just ensure running
                m_proactiveListener->start();
            } else {
                // Mode changed: recreate listener
                m_proactiveListener->stop();
                delete m_proactiveListener;
                m_proactiveListener = nullptr;
            }
        }

        if (!m_proactiveListener) {
            QSerialPort* serialPort = m_busController->underlyingSerialPort();
            if (!serialPort) {
                m_lastError = "Bus controller does not provide underlying serial port access";
                emit errorOccurred(m_lastError);
                return false;
            }
            m_proactiveListener = new EncoderProactiveListener(serialPort, listenerMode, m_resolution, this);
        }
        m_proactiveListener->start();

        qDebug() << "Encoder auto-report mode set to" << mode
                 << "(communicationMode=" << m_communicationMode << ")"
                 << "interval=" << intervalMs << "ms";
    }

    return true;
}

QString SingleTurnEncoderDevice::lastError() const {
    return m_lastError;
}

bool SingleTurnEncoderDevice::readRegisters(uint16_t address, uint16_t count, QVector<uint16_t>& values) {
    QByteArray request = Bus::ModbusFrame::buildReadHoldingRegisters(m_slaveId, address, count);

    for (int attempt = 0; attempt < MAX_RETRIES; ++attempt) {
        QByteArray response;

        if (!m_busController->sendRequest(request, response)) {
            m_lastError = m_busController->lastError();

            if (attempt < MAX_RETRIES - 1) {
                qWarning() << QString("Encoder read registers failed (attempt %1/%2): %3. Retrying...")
                              .arg(attempt + 1).arg(MAX_RETRIES).arg(m_lastError);
                QThread::msleep(RETRY_DELAY_MS);
                continue;
            }

            qCritical() << QString("Encoder read registers failed after %1 attempts: %2")
                           .arg(MAX_RETRIES).arg(m_lastError);
            emit errorOccurred(m_lastError);
            return false;
        }

        if (!Bus::ModbusFrame::parseReadHoldingRegistersResponse(response, count, values)) {
            // Check if it's an exception response
            uint8_t functionCode = static_cast<uint8_t>(response[1]);
            if (functionCode & 0x80) {
                QPair<uint8_t, QString> exception = Bus::ModbusFrame::parseExceptionResponse(response);
                m_lastError = QString("Modbus exception: %1").arg(exception.second);
            } else {
                m_lastError = "Invalid read response or CRC error";
            }

            if (attempt < MAX_RETRIES - 1) {
                qWarning() << QString("Encoder parse read response failed (attempt %1/%2): %3. Retrying...")
                              .arg(attempt + 1).arg(MAX_RETRIES).arg(m_lastError);
                QThread::msleep(RETRY_DELAY_MS);
                continue;
            }

            qCritical() << QString("Encoder parse read response failed after %1 attempts: %2")
                           .arg(MAX_RETRIES).arg(m_lastError);
            emit errorOccurred(m_lastError);
            return false;
        }

        // Success
        if (attempt > 0) {
            qDebug() << QString("Encoder read registers succeeded on attempt %1").arg(attempt + 1);
        }
        return true;
    }

    return false;
}

bool SingleTurnEncoderDevice::writeRegister(uint16_t address, uint16_t value) {
    QByteArray request = Bus::ModbusFrame::buildWriteSingleRegister(m_slaveId, address, value);

    for (int attempt = 0; attempt < MAX_RETRIES; ++attempt) {
        QByteArray response;

        if (!m_busController->sendRequest(request, response)) {
            m_lastError = m_busController->lastError();

            if (attempt < MAX_RETRIES - 1) {
                qWarning() << QString("Encoder write register failed (attempt %1/%2): %3. Retrying...")
                              .arg(attempt + 1).arg(MAX_RETRIES).arg(m_lastError);
                QThread::msleep(RETRY_DELAY_MS);
                continue;
            }

            qCritical() << QString("Encoder write register failed after %1 attempts: %2")
                           .arg(MAX_RETRIES).arg(m_lastError);
            emit errorOccurred(m_lastError);
            return false;
        }

        if (!Bus::ModbusFrame::parseWriteSingleRegisterResponse(response, address, value)) {
            // Check if it's an exception response
            uint8_t functionCode = static_cast<uint8_t>(response[1]);
            if (functionCode & 0x80) {
                QPair<uint8_t, QString> exception = Bus::ModbusFrame::parseExceptionResponse(response);
                m_lastError = QString("Modbus exception: %1").arg(exception.second);
            } else {
                m_lastError = "Invalid write response or CRC error";
            }

            if (attempt < MAX_RETRIES - 1) {
                qWarning() << QString("Encoder parse write response failed (attempt %1/%2): %3. Retrying...")
                              .arg(attempt + 1).arg(MAX_RETRIES).arg(m_lastError);
                QThread::msleep(RETRY_DELAY_MS);
                continue;
            }

            qCritical() << QString("Encoder parse write response failed after %1 attempts: %2")
                           .arg(MAX_RETRIES).arg(m_lastError);
            emit errorOccurred(m_lastError);
            return false;
        }

        // Success
        if (attempt > 0) {
            qDebug() << QString("Encoder write register succeeded on attempt %1").arg(attempt + 1);
        }
        return true;
    }

    return false;
}

bool SingleTurnEncoderDevice::setResolution(uint16_t resolution) {
    if (!writeRegister(REG_RESOLUTION, resolution)) {
        m_lastError = QString("Failed to set resolution: %1").arg(m_lastError);
        emit errorOccurred(m_lastError);
        return false;
    }

    m_resolution = resolution;

    // Update listener resolution if active
    if (m_proactiveListener) {
        m_proactiveListener->setResolution(resolution);
    }

    qDebug() << "Encoder resolution set to" << resolution;
    return true;
}

uint16_t SingleTurnEncoderDevice::getResolution() const {
    return m_resolution;
}

int SingleTurnEncoderDevice::getCommunicationMode() const {
    return m_communicationMode;
}

} // namespace Devices
} // namespace Infrastructure
