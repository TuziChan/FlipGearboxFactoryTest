#include "MockTorqueDevice.h"

namespace Infrastructure {
namespace Simulation {

MockTorqueDevice::MockTorqueDevice(uint8_t slaveId, QObject* parent)
    : MockModbusDevice(slaveId, parent)
    , m_simulatedTorqueNm(0.0)
    , m_simulatedSpeedRpm(0.0)
    , m_simulatedPowerW(0.0)
{
    // Initialize communication mode to Modbus RTU
    setHoldingRegister(REG_COMM_MODE, 0);

    // Initialize 32-bit registers
    set32BitRegister(REG_TORQUE_HIGH, REG_TORQUE_LOW, 0);
    set32BitRegister(REG_SPEED_HIGH, REG_SPEED_LOW, 0);
    set32BitRegister(REG_POWER_HIGH, REG_POWER_LOW, 0);
}

void MockTorqueDevice::setSimulatedTorque(double torqueNm) {
    m_simulatedTorqueNm = torqueNm;
}

void MockTorqueDevice::setSimulatedSpeed(double speedRpm) {
    m_simulatedSpeedRpm = speedRpm;
}

void MockTorqueDevice::setSimulatedPower(double powerW) {
    m_simulatedPowerW = powerW;
}

void MockTorqueDevice::updateDynamicRegisters() {
    // Update torque (×100 for 0.01 N·m resolution)
    int32_t torqueRaw = static_cast<int32_t>(m_simulatedTorqueNm * 100.0);
    set32BitRegister(REG_TORQUE_HIGH, REG_TORQUE_LOW, torqueRaw);

    // Update speed (×1 RPM, no scaling)
    int32_t speedRaw = static_cast<int32_t>(m_simulatedSpeedRpm);
    set32BitRegister(REG_SPEED_HIGH, REG_SPEED_LOW, speedRaw);

    // Update power (×10 for 0.1 W resolution)
    int32_t powerRaw = static_cast<int32_t>(m_simulatedPowerW * 10.0);
    set32BitRegister(REG_POWER_HIGH, REG_POWER_LOW, powerRaw);
}

void MockTorqueDevice::set32BitRegister(uint16_t highAddr, uint16_t lowAddr, int32_t value) {
    // Big-endian: high word first
    uint32_t uValue = static_cast<uint32_t>(value);
    uint16_t highWord = static_cast<uint16_t>((uValue >> 16) & 0xFFFF);
    uint16_t lowWord = static_cast<uint16_t>(uValue & 0xFFFF);
    
    setHoldingRegister(highAddr, highWord);
    setHoldingRegister(lowAddr, lowWord);
}

} // namespace Simulation
} // namespace Infrastructure
