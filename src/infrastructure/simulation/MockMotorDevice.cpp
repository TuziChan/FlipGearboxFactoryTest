#include "MockMotorDevice.h"

namespace Infrastructure {
namespace Simulation {

MockMotorDevice::MockMotorDevice(uint8_t slaveId, QObject* parent)
    : MockModbusDevice(slaveId, parent)
    , m_simulatedCurrentA(0.0)
    , m_ai1InputLevel(false)
    , m_isBraking(false)
{
    // Initialize device ID
    setHoldingRegister(REG_DEVICE_ID, 0x3610); // AQMD3610

    // Initialize registers
    setHoldingRegister(REG_REAL_TIME_CURRENT, 0);
    setHoldingRegister(REG_SET_SPEED, 0);
    setHoldingRegister(REG_STOP_AND_LOCK, 0);
    setHoldingRegister(REG_NATURAL_STOP, 0);
    setHoldingRegister(REG_AI1_PORT_DIRECTION, 0); // Input mode
    setHoldingRegister(REG_AI1_PORT_LEVEL, 0);
}

void MockMotorDevice::setSimulatedCurrent(double currentA) {
    m_simulatedCurrentA = currentA;
}

void MockMotorDevice::setAI1InputLevel(bool high) {
    m_ai1InputLevel = high;
}

int16_t MockMotorDevice::getMotorSpeed() const {
    uint16_t rawValue = getHoldingRegister(REG_SET_SPEED);
    return static_cast<int16_t>(rawValue);
}

bool MockMotorDevice::isBraking() const {
    return m_isBraking;
}

void MockMotorDevice::onRegisterWrite(uint16_t address, uint16_t value) {
    switch (address) {
    case REG_SET_SPEED:
        // Motor speed set, update current simulation
        {
            int16_t speed = static_cast<int16_t>(value);
            // Simulate current proportional to speed
            m_simulatedCurrentA = qAbs(speed) * 0.005; // Max ~5A at full speed
            m_isBraking = false;
        }
        break;

    case REG_STOP_AND_LOCK:
        if (value == 1) {
            m_isBraking = true;
            setHoldingRegister(REG_SET_SPEED, 0);
            m_simulatedCurrentA = 0.0;
        }
        break;

    case REG_NATURAL_STOP:
        if (value == 1) {
            m_isBraking = false;
            setHoldingRegister(REG_SET_SPEED, 0);
            m_simulatedCurrentA = 0.0;
        }
        break;

    case REG_AI1_PORT_DIRECTION:
        // Port direction changed
        break;

    default:
        break;
    }
}

void MockMotorDevice::updateDynamicRegisters() {
    // Update real-time current (×100 for 0.01A resolution)
    uint16_t currentRaw = static_cast<uint16_t>(m_simulatedCurrentA * 100.0);
    setHoldingRegister(REG_REAL_TIME_CURRENT, currentRaw);

    // Update AI1 level based on direction
    uint16_t direction = getHoldingRegister(REG_AI1_PORT_DIRECTION);
    if (direction == 0) {
        // Input mode: use simulated input level
        setHoldingRegister(REG_AI1_PORT_LEVEL, m_ai1InputLevel ? 1 : 0);
    }
    // Output mode: level is controlled by writes
}

} // namespace Simulation
} // namespace Infrastructure
