#include "SingleTurnEncoderDevice.h"
#include "../bus/ModbusFrame.h"
#include <QDebug>

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

bool SingleTurnEncoderDevice::initialize() {
    if (!m_busController || !m_busController->isOpen()) {
        m_lastError = "Bus controller is not open";
        return false;
    }

    if (m_communicationMode == 0) {
        // Mode 0: Query mode (default)
        // Try reading angle to verify communication
        double angle;
        if (!readAngle(angle)) {
            m_lastError = QString("Failed to verify encoder communication: %1").arg(m_lastError);
            return false;
        }

        qDebug() << "Single-turn encoder initialized in query mode, slave ID:" << m_slaveId 
                 << "Resolution:" << m_resolution
                 << "Current angle:" << angle << "deg";
    } else {
        // Modes 2, 3, 4: Auto-report modes
        uint16_t modeValue;
        QString modeName;
        
        switch (m_communicationMode) {
            case 2: // Auto-report single-turn value
                modeValue = 0x01;
                modeName = "auto-report single-turn";
                break;
            case 3: // Auto-report virtual multi-turn value
                modeValue = 0x04;
                modeName = "auto-report virtual multi-turn";
                break;
            case 4: // Auto-report angular velocity
                modeValue = 0x05;
                modeName = "auto-report angular velocity";
                break;
            default:
                m_lastError = QString("Invalid communication mode: %1").arg(m_communicationMode);
                return false;
        }
        
        // Write auto-report mode to register 0x0006
        if (!writeRegister(REG_AUTO_REPORT_MODE, modeValue)) {
            m_lastError = QString("Failed to set encoder auto-report mode: %1").arg(m_lastError);
            return false;
        }
        
        // Write auto-report interval to register 0x0007
        if (!writeRegister(REG_AUTO_REPORT_INTERVAL, static_cast<uint16_t>(m_autoReportIntervalMs))) {
            m_lastError = QString("Failed to set encoder auto-report interval: %1").arg(m_lastError);
            return false;
        }
        
        // Get underlying serial port for proactive listener
        QSerialPort* serialPort = m_busController->underlyingSerialPort();
        if (!serialPort) {
            m_lastError = "Bus controller does not provide underlying serial port access";
            return false;
        }
        
        // Create and start proactive listener
        m_proactiveListener = new EncoderProactiveListener(serialPort, this);
        m_proactiveListener->start();
        
        qDebug() << "Single-turn encoder initialized in" << modeName << "mode, slave ID:" << m_slaveId
                 << "Resolution:" << m_resolution
                 << "Interval:" << m_autoReportIntervalMs << "ms"
                 << "Proactive listener started";
    }

    return true;
}

bool SingleTurnEncoderDevice::readAngle(double& angleDeg) {
    // In proactive mode, get data from listener instead of polling
    if (m_proactiveListener && m_proactiveListener->isValid()) {
        angleDeg = m_proactiveListener->latestAngle();
        return true;
    }
    
    // In query mode (mode 0), use Modbus polling
    QVector<uint16_t> values;
    if (!readRegisters(REG_ANGLE, 1, values)) {
        m_lastError = QString("Failed to read angle: %1").arg(m_lastError);
        return false;
    }

    // Convert raw count to angle: angleDeg = (count / resolution) × 360°
    uint16_t count = values[0];
    angleDeg = (static_cast<double>(count) / m_resolution) * 360.0;
    return true;
}

bool SingleTurnEncoderDevice::readVirtualMultiTurn(double& totalAngleDeg) {
    // Read 2 registers (0x0000-0x0001) for 32-bit multi-turn count
    QVector<uint16_t> values;
    if (!readRegisters(REG_VIRTUAL_MULTITURN, 2, values)) {
        m_lastError = QString("Failed to read virtual multi-turn: %1").arg(m_lastError);
        return false;
    }

    // Combine to uint32 (big-endian: high word first)
    uint32_t count = (static_cast<uint32_t>(values[0]) << 16) | static_cast<uint32_t>(values[1]);
    
    // Convert to total angle: totalAngleDeg = (count / resolution) × 360°
    totalAngleDeg = (static_cast<double>(count) / m_resolution) * 360.0;
    return true;
}

bool SingleTurnEncoderDevice::readAngularVelocity(double& velocityRpm) {
    // Read 1 register (0x0003) for signed 16-bit velocity
    QVector<uint16_t> values;
    if (!readRegisters(REG_ANGULAR_VELOCITY, 1, values)) {
        m_lastError = QString("Failed to read angular velocity: %1").arg(m_lastError);
        return false;
    }

    // Interpret as signed int16
    int16_t velocityRaw = static_cast<int16_t>(values[0]);
    
    // Convert to RPM: rpm = value / resolution / (sampleTimeMs / 60000.0)
    // Assuming sample time is the auto-report interval
    double sampleTimeSec = m_autoReportIntervalMs / 1000.0;
    if (sampleTimeSec > 0) {
        velocityRpm = (static_cast<double>(velocityRaw) / m_resolution) * (60.0 / sampleTimeSec);
    } else {
        velocityRpm = 0.0;
    }
    
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
    // Write auto-report mode to register 0x0006
    if (!writeRegister(REG_AUTO_REPORT_MODE, mode)) {
        m_lastError = QString("Failed to set auto-report mode: %1").arg(m_lastError);
        return false;
    }
    
    // Write auto-report interval to register 0x0007
    if (!writeRegister(REG_AUTO_REPORT_INTERVAL, static_cast<uint16_t>(intervalMs))) {
        m_lastError = QString("Failed to set auto-report interval: %1").arg(m_lastError);
        return false;
    }
    
    m_autoReportIntervalMs = intervalMs;
    
    // Start or stop proactive listener based on mode
    if (mode == 0x00) {
        // Mode 0: Query mode - stop listener
        if (m_proactiveListener) {
            m_proactiveListener->stop();
            m_proactiveListener->deleteLater();
            m_proactiveListener = nullptr;
        }
        qDebug() << "Encoder switched to query mode";
    } else {
        // Auto-report mode - start listener if not already running
        if (!m_proactiveListener) {
            QSerialPort* serialPort = m_busController->underlyingSerialPort();
            if (!serialPort) {
                m_lastError = "Bus controller does not provide underlying serial port access";
                return false;
            }
            m_proactiveListener = new EncoderProactiveListener(serialPort, this);
            m_proactiveListener->start();
        }
        qDebug() << "Encoder auto-report mode set to" << mode << "with interval" << intervalMs << "ms";
    }
    
    return true;
}

QString SingleTurnEncoderDevice::lastError() const {
    return m_lastError;
}

bool SingleTurnEncoderDevice::readRegisters(uint16_t address, uint16_t count, QVector<uint16_t>& values) {
    QByteArray request = Bus::ModbusFrame::buildReadHoldingRegisters(m_slaveId, address, count);
    QByteArray response;
    
    if (!m_busController->sendRequest(request, response)) {
        m_lastError = m_busController->lastError();
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
        return false;
    }

    return true;
}

bool SingleTurnEncoderDevice::writeRegister(uint16_t address, uint16_t value) {
    QByteArray request = Bus::ModbusFrame::buildWriteSingleRegister(m_slaveId, address, value);
    QByteArray response;
    
    if (!m_busController->sendRequest(request, response)) {
        m_lastError = m_busController->lastError();
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
        return false;
    }

    return true;
}

} // namespace Devices
} // namespace Infrastructure
