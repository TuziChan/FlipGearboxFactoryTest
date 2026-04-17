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
    int16_t registerValue = 0;
    
    switch (direction) {
        case Direction::Forward:
            registerValue = static_cast<int16_t>(dutyCyclePercent * 10.0);
            break;
        case Direction::Reverse:
            registerValue = -static_cast<int16_t>(dutyCyclePercent * 10.0);
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
        m_lastError = "Invalid write response or CRC error";
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
        m_lastError = "Invalid write response or CRC error";
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
        m_lastError = "Invalid read response or CRC error";
        return false;
    }

    return true;
}

} // namespace Devices
} // namespace Infrastructure
