#include "Dyn200TorqueSensorDevice.h"
#include "../bus/ModbusFrame.h"
#include <QDebug>

namespace Infrastructure {
namespace Devices {

Dyn200TorqueSensorDevice::Dyn200TorqueSensorDevice(Bus::IBusController* busController, 
                                                     uint8_t slaveId,
                                                     QObject* parent)
    : ITorqueSensorDevice(parent)
    , m_busController(busController)
    , m_slaveId(slaveId)
    , m_lastError()
{
}

bool Dyn200TorqueSensorDevice::initialize() {
    if (!m_busController || !m_busController->isOpen()) {
        m_lastError = "Bus controller is not open";
        return false;
    }

    // Try reading torque to verify communication
    double torque;
    if (!readTorque(torque)) {
        m_lastError = QString("Failed to verify DYN200 communication: %1").arg(m_lastError);
        return false;
    }

    qDebug() << "DYN200 torque sensor initialized, slave ID:" << m_slaveId;
    return true;
}

bool Dyn200TorqueSensorDevice::readTorque(double& torqueNm) {
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
    // Read all 6 registers at once for efficiency
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
        m_lastError = "Invalid read response or CRC error";
        return false;
    }

    return true;
}

int32_t Dyn200TorqueSensorDevice::combineToInt32(uint16_t highWord, uint16_t lowWord) const {
    // Big-endian: high word first
    uint32_t combined = (static_cast<uint32_t>(highWord) << 16) | static_cast<uint32_t>(lowWord);
    return static_cast<int32_t>(combined);
}

} // namespace Devices
} // namespace Infrastructure
