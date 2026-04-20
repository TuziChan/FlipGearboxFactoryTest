#include "Dyn200TorqueSensorDevice.h"
#include "../bus/ModbusFrame.h"
#include "../bus/ModbusRtuBusController.h"
#include <QDebug>

namespace Infrastructure {
namespace Devices {

Dyn200TorqueSensorDevice::Dyn200TorqueSensorDevice(Bus::IBusController* busController, 
                                                     uint8_t slaveId,
                                                     int communicationMode,
                                                     QObject* parent)
    : ITorqueSensorDevice(parent)
    , m_busController(busController)
    , m_slaveId(slaveId)
    , m_communicationMode(communicationMode)
    , m_lastError()
    , m_proactiveListener(nullptr)
{
}

Dyn200TorqueSensorDevice::~Dyn200TorqueSensorDevice() {
    if (m_proactiveListener) {
        m_proactiveListener->stop();
        delete m_proactiveListener;
    }
}

bool Dyn200TorqueSensorDevice::initialize() {
    if (!m_busController || !m_busController->isOpen()) {
        m_lastError = "Bus controller is not open";
        return false;
    }

    if (m_communicationMode == 0) {
        // Mode 0: Modbus RTU polling (default)
        // Try reading torque to verify communication
        double torque;
        if (!readTorque(torque)) {
            m_lastError = QString("Failed to verify DYN200 communication: %1").arg(m_lastError);
            return false;
        }
        
        qDebug() << "DYN200 torque sensor initialized in Modbus RTU mode, slave ID:" << m_slaveId;
    } else {
        // Modes 1, 2, 3: Proactive upload modes
        // First, switch the device to the desired proactive mode via Modbus
        uint16_t modeValue;
        Dyn200ProactiveListener::ProtocolMode listenerMode;
        
        switch (m_communicationMode) {
            case 1: // HEX 6-byte
                modeValue = 0;
                listenerMode = Dyn200ProactiveListener::Hex6Byte;
                qDebug() << "Switching DYN200 to HEX 6-byte proactive upload mode";
                break;
            case 2: // HEX 8-byte
                modeValue = 3;
                listenerMode = Dyn200ProactiveListener::Hex8Byte;
                qDebug() << "Switching DYN200 to HEX 8-byte proactive upload mode";
                break;
            case 3: // ASCII
                modeValue = 2;
                listenerMode = Dyn200ProactiveListener::Ascii;
                qDebug() << "Switching DYN200 to ASCII proactive upload mode";
                break;
            default:
                m_lastError = QString("Invalid communication mode: %1").arg(m_communicationMode);
                return false;
        }
        
        // Write to register 0x1CH to switch mode
        if (!writeRegister(REG_COMM_MODE, modeValue)) {
            m_lastError = QString("Failed to switch DYN200 to proactive mode: %1").arg(m_lastError);
            return false;
        }
        
        qDebug() << "DYN200 switched to proactive mode, register 0x1CH =" << modeValue;
        qDebug() << "WARNING: To switch back to Modbus RTU mode, manual factory reset is required via sensor buttons";
        
        // Get the serial port from the bus controller
        QSerialPort* serialPort = getSerialPort();
        if (!serialPort) {
            m_lastError = "Failed to get serial port from bus controller";
            return false;
        }
        
        // Create and start the proactive listener
        m_proactiveListener = new Dyn200ProactiveListener(listenerMode, this);
        m_proactiveListener->start(serialPort);
        
        qDebug() << "DYN200 proactive listener started";
    }

    return true;
}

bool Dyn200TorqueSensorDevice::readTorque(double& torqueNm) {
    if (m_proactiveListener) {
        // Proactive mode: read from cached value
        torqueNm = m_proactiveListener->latestTorque();
        return true;
    }
    
    // Modbus RTU mode: read from registers
    QVector<uint16_t> values;
    if (!readRegisters(REG_TORQUE, 2, values)) {
        m_lastError = QString("Failed to read torque: %1").arg(m_lastError);
        return false;
    }

    int32_t rawValue = combineToInt32(values[0], values[1]);
    torqueNm = rawValue * 0.01; // Scale: ×0.01 N·m
    return true;
}

bool Dyn200TorqueSensorDevice::readSpeed(double& speedRpm) {
    if (m_proactiveListener) {
        // Proactive mode: read from cached value
        if (!m_proactiveListener->isSpeedValid()) {
            m_lastError = "Speed not available in ASCII mode";
            return false;
        }
        speedRpm = m_proactiveListener->latestSpeed();
        return true;
    }
    
    // Modbus RTU mode: read from registers
    QVector<uint16_t> values;
    if (!readRegisters(REG_SPEED, 2, values)) {
        m_lastError = QString("Failed to read speed: %1").arg(m_lastError);
        return false;
    }

    int32_t rawValue = combineToInt32(values[0], values[1]);
    speedRpm = static_cast<double>(rawValue); // Scale: ×1 RPM (NO scaling)
    return true;
}

bool Dyn200TorqueSensorDevice::readPower(double& powerW) {
    if (m_proactiveListener) {
        // Power not available in proactive modes
        m_lastError = "Power not available in proactive upload modes";
        return false;
    }
    
    // Modbus RTU mode: read from registers
    QVector<uint16_t> values;
    if (!readRegisters(REG_POWER, 2, values)) {
        m_lastError = QString("Failed to read power: %1").arg(m_lastError);
        return false;
    }

    int32_t rawValue = combineToInt32(values[0], values[1]);
    powerW = rawValue * 0.1; // Scale: ×0.1 W
    return true;
}

bool Dyn200TorqueSensorDevice::readAll(double& torqueNm, double& speedRpm, double& powerW) {
    if (m_proactiveListener) {
        // Proactive mode: read from cached values
        torqueNm = m_proactiveListener->latestTorque();
        
        if (m_proactiveListener->isSpeedValid()) {
            speedRpm = m_proactiveListener->latestSpeed();
        } else {
            speedRpm = 0.0;
        }
        
        powerW = 0.0; // Power not available in proactive modes
        return true;
    }
    
    // Modbus RTU mode: read all 6 registers at once for efficiency
    QVector<uint16_t> values;
    if (!readRegisters(REG_TORQUE, 6, values)) {
        m_lastError = QString("Failed to read all telemetry: %1").arg(m_lastError);
        return false;
    }

    int32_t torqueRaw = combineToInt32(values[0], values[1]);
    int32_t speedRaw = combineToInt32(values[2], values[3]);
    int32_t powerRaw = combineToInt32(values[4], values[5]);

    torqueNm = torqueRaw * 0.01;  // ×0.01 N·m
    speedRpm = static_cast<double>(speedRaw);  // ×1 RPM (NO scaling)
    powerW = powerRaw * 0.1;  // ×0.1 W

    return true;
}

QString Dyn200TorqueSensorDevice::lastError() const {
    return m_lastError;
}

bool Dyn200TorqueSensorDevice::readRegisters(uint16_t address, uint16_t count, QVector<uint16_t>& values) {
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

bool Dyn200TorqueSensorDevice::writeRegister(uint16_t address, uint16_t value) {
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

int32_t Dyn200TorqueSensorDevice::combineToInt32(uint16_t highWord, uint16_t lowWord) const {
    // Big-endian: high word first
    uint32_t combined = (static_cast<uint32_t>(highWord) << 16) | static_cast<uint32_t>(lowWord);
    return static_cast<int32_t>(combined);
}

QSerialPort* Dyn200TorqueSensorDevice::getSerialPort() const {
// Use the abstract interface method to get the serial port
return m_busController->underlyingSerialPort();
}

} // namespace Devices
} // namespace Infrastructure
