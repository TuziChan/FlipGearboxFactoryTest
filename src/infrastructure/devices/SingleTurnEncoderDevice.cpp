#include "SingleTurnEncoderDevice.h"
#include "../bus/ModbusFrame.h"
#include <QDebug>

namespace Infrastructure {
namespace Devices {

SingleTurnEncoderDevice::SingleTurnEncoderDevice(Bus::IBusController* busController, 
                                                   uint8_t slaveId,
                                                   uint16_t resolution,
                                                   QObject* parent)
    : IEncoderDevice(parent)
    , m_busController(busController)
    , m_slaveId(slaveId)
    , m_resolution(resolution)
    , m_lastError()
{
}

bool SingleTurnEncoderDevice::initialize() {
    if (!m_busController || !m_busController->isOpen()) {
        m_lastError = "Bus controller is not open";
        return false;
    }

    // Try reading angle to verify communication
    double angle;
    if (!readAngle(angle)) {
        m_lastError = QString("Failed to verify encoder communication: %1").arg(m_lastError);
        return false;
    }

    qDebug() << "Single-turn encoder initialized, slave ID:" << m_slaveId 
             << "Resolution:" << m_resolution
             << "Current angle:" << angle << "deg";
    return true;
}

bool SingleTurnEncoderDevice::readAngle(double& angleDeg) {
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

bool SingleTurnEncoderDevice::setZeroPoint() {
    if (!writeRegister(REG_SET_ZERO, 1)) {
        m_lastError = QString("Failed to set zero point: %1").arg(m_lastError);
        return false;
    }

    qDebug() << "Encoder zero point set";
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
        m_lastError = "Invalid read response or CRC error";
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
        m_lastError = "Invalid write response or CRC error";
        return false;
    }

    return true;
}

} // namespace Devices
} // namespace Infrastructure
