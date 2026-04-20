#include "AqmdMotorDriveDevice.h"
#include "../bus/ModbusFrame.h"
#include <QDebug>

namespace Infrastructure {
namespace Devices {

AqmdMotorDriveDevice::AqmdMotorDriveDevice(Bus::IBusController* busController, 
                                             uint8_t slaveId,
                                             QObject* parent)
    : IMotorDriveDevice(parent)
    , m_busController(busController)
    , m_slaveId(slaveId)
    , m_lastError()
{
}

bool AqmdMotorDriveDevice::initialize() {
    if (!m_busController || !m_busController->isOpen()) {
        m_lastError = "Bus controller is not open";
        return false;
    }

    // Read device ID to verify communication
    QVector<uint16_t> values;
    if (!readRegisters(REG_DEVICE_ID, 1, values)) {
        m_lastError = QString("Failed to read device ID: %1").arg(m_lastError);
        return false;
    }

    // Configure AI1 as input for magnet detection
    if (!writeRegister(REG_AI1_PORT_DIRECTION, 0)) {
        m_lastError = QString("Failed to configure AI1 as input: %1").arg(m_lastError);
        return false;
    }

    qDebug() << "AQMD motor drive initialized, slave ID:" << m_slaveId 
             << "Device ID:" << QString::number(values[0], 16);
    
    return true;
}

bool AqmdMotorDriveDevice::setMotor(Direction direction, double dutyCyclePercent) {
    // Clamp duty cycle to valid range
    dutyCyclePercent = qBound(0.0, dutyCyclePercent, 100.0);
    
    // Convert to register value: -1000 to +1000 (×0.1%)
    // Check for overflow before conversion
    double rawValue = dutyCyclePercent * 10.0;
    
    int16_t registerValue = 0;
    
    switch (direction) {
        case Direction::Forward:
            // Check for int16_t overflow
            if (rawValue > 1000.0) {
                qWarning() << "Duty cycle value" << rawValue << "exceeds max 1000, clamping";
                registerValue = 1000;
            } else {
                registerValue = static_cast<int16_t>(rawValue);
            }
            break;
        case Direction::Reverse:
            // Check for int16_t underflow
            if (rawValue > 1000.0) {
                qWarning() << "Duty cycle value" << rawValue << "exceeds max 1000, clamping";
                registerValue = -1000;
            } else {
                registerValue = -static_cast<int16_t>(rawValue);
            }
            break;
        case Direction::Brake:
            registerValue = 0;
            break;
    }

    if (!writeRegisterSigned(REG_SET_SPEED, registerValue)) {
        m_lastError = QString("Failed to set motor speed: %1").arg(m_lastError);
        return false;
    }

    return true;
}

bool AqmdMotorDriveDevice::brake() {
    return setMotor(Direction::Brake, 0.0);
}

bool AqmdMotorDriveDevice::coast() {
    // Write 1 to natural stop register
    if (!writeRegister(REG_NATURAL_STOP, 1)) {
        m_lastError = QString("Failed to coast motor: %1").arg(m_lastError);
        return false;
    }
    return true;
}

bool AqmdMotorDriveDevice::readCurrent(double& currentA) {
    QVector<uint16_t> values;
    if (!readRegisters(REG_REAL_TIME_CURRENT, 1, values)) {
        m_lastError = QString("Failed to read current: %1").arg(m_lastError);
        return false;
    }

    // Scale: ×0.01A
    currentA = values[0] * 0.01;
    return true;
}

bool AqmdMotorDriveDevice::readAI1Level(bool& level) {
    QVector<uint16_t> values;
    if (!readRegisters(REG_AI1_PORT_LEVEL, 1, values)) {
        m_lastError = QString("Failed to read AI1 level: %1").arg(m_lastError);
        return false;
    }

    // 0 = low (magnet detected), 1 = high (no magnet)
    level = (values[0] != 0);
    return true;
}

bool AqmdMotorDriveDevice::readDeviceIdentification(QString& vendor, QString& product, QString& version) {
    QByteArray request = Bus::ModbusFrame::buildReadDeviceIdentification(m_slaveId);
    QByteArray response;
    
    if (!m_busController->sendRequest(request, response)) {
        m_lastError = m_busController->lastError();
        return false;
    }

    if (!Bus::ModbusFrame::parseReadDeviceIdentificationResponse(response, vendor, product, version)) {
        // Check if it's an exception response
        uint8_t functionCode = static_cast<uint8_t>(response[1]);
        if (functionCode & 0x80) {
            QPair<uint8_t, QString> exception = Bus::ModbusFrame::parseExceptionResponse(response);
            m_lastError = QString("Modbus exception: %1").arg(exception.second);
        } else {
            m_lastError = "Invalid device identification response or CRC error";
        }
        return false;
    }

    return true;
}

bool AqmdMotorDriveDevice::writeMultipleRegisters(uint16_t address, const QVector<uint16_t>& values) {
    QByteArray request = Bus::ModbusFrame::buildWriteMultipleRegisters(m_slaveId, address, values);
    QByteArray response;
    
    if (!m_busController->sendRequest(request, response)) {
        m_lastError = m_busController->lastError();
        return false;
    }

    if (!Bus::ModbusFrame::parseWriteMultipleRegistersResponse(response, address, static_cast<uint16_t>(values.size()))) {
        // Check if it's an exception response
        uint8_t functionCode = static_cast<uint8_t>(response[1]);
        if (functionCode & 0x80) {
            QPair<uint8_t, QString> exception = Bus::ModbusFrame::parseExceptionResponse(response);
            m_lastError = QString("Modbus exception: %1").arg(exception.second);
        } else {
            m_lastError = "Invalid write multiple registers response or CRC error";
        }
        return false;
    }

    qDebug() << "AQMD: Wrote" << values.size() << "registers starting at address" << QString::number(address, 16);
    
    // Note: Writing to GPIO registers (0x0050-0x0053) with 0x10 triggers EEPROM storage
    if (address >= 0x0050 && address <= 0x0053) {
        qDebug() << "AQMD: GPIO configuration written, EEPROM storage triggered";
    }

    return true;
}

QString AqmdMotorDriveDevice::lastError() const {
    return m_lastError;
}

bool AqmdMotorDriveDevice::writeRegister(uint16_t address, uint16_t value) {
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

bool AqmdMotorDriveDevice::writeRegisterSigned(uint16_t address, int16_t value) {
    QByteArray request = Bus::ModbusFrame::buildWriteSingleRegisterSigned(m_slaveId, address, value);
    QByteArray response;
    
    if (!m_busController->sendRequest(request, response)) {
        m_lastError = m_busController->lastError();
        return false;
    }

    uint16_t unsignedValue = static_cast<uint16_t>(value);
    if (!Bus::ModbusFrame::parseWriteSingleRegisterResponse(response, address, unsignedValue)) {
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

bool AqmdMotorDriveDevice::readRegisters(uint16_t address, uint16_t count, QVector<uint16_t>& values) {
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

} // namespace Devices
} // namespace Infrastructure
