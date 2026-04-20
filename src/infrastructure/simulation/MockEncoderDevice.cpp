#include "MockEncoderDevice.h"

namespace Infrastructure {
namespace Simulation {

MockEncoderDevice::MockEncoderDevice(uint8_t slaveId, uint16_t resolution, QObject* parent)
    : MockModbusDevice(slaveId, parent)
    , m_resolution(resolution)
    , m_simulatedAngleDeg(0.0)
    , m_simulatedVelocityRpm(0.0)
    , m_zeroOffset(0)
    , m_virtualMultiTurnCount(0)
{
    // Initialize registers
    setHoldingRegister(REG_ANGLE, 0);
    setHoldingRegister(REG_VIRTUAL_MULTITURN_HIGH, 0);
    setHoldingRegister(REG_VIRTUAL_MULTITURN_LOW, 0);
    setHoldingRegister(REG_ANGULAR_VELOCITY, 0);
    setHoldingRegister(REG_AUTO_REPORT_MODE, 0);
    setHoldingRegister(REG_AUTO_REPORT_INTERVAL, 20);
    setHoldingRegister(REG_SET_ZERO, 0);
}

void MockEncoderDevice::setSimulatedAngle(double angleDeg) {
    m_simulatedAngleDeg = angleDeg;
    
    // Normalize to 0-360
    while (m_simulatedAngleDeg < 0.0) {
        m_simulatedAngleDeg += 360.0;
    }
    while (m_simulatedAngleDeg >= 360.0) {
        m_simulatedAngleDeg -= 360.0;
    }
}

void MockEncoderDevice::setSimulatedVelocity(double velocityRpm) {
    m_simulatedVelocityRpm = velocityRpm;
}

void MockEncoderDevice::onRegisterWrite(uint16_t address, uint16_t value) {
    switch (address) {
    case REG_SET_ZERO:
        if (value == 1) {
            // Set current position as zero point
            uint16_t currentCount = static_cast<uint16_t>((m_simulatedAngleDeg / 360.0) * m_resolution);
            m_zeroOffset = currentCount;
            m_virtualMultiTurnCount = 0;
            
            // Clear the register after processing
            setHoldingRegister(REG_SET_ZERO, 0);
        }
        break;

    case REG_AUTO_REPORT_MODE:
        // Auto-report mode changed
        break;

    case REG_AUTO_REPORT_INTERVAL:
        // Auto-report interval changed
        break;

    default:
        break;
    }
}

void MockEncoderDevice::updateDynamicRegisters() {
    // Calculate raw count from angle
    uint16_t rawCount = static_cast<uint16_t>((m_simulatedAngleDeg / 360.0) * m_resolution);
    
    // Apply zero offset
    int32_t adjustedCount = static_cast<int32_t>(rawCount) - static_cast<int32_t>(m_zeroOffset);
    if (adjustedCount < 0) {
        adjustedCount += m_resolution;
    }
    
    // Update single-turn register
    setHoldingRegister(REG_ANGLE, static_cast<uint16_t>(adjustedCount));
    
    // Update virtual multi-turn (32-bit big-endian)
    uint32_t multiTurnValue = static_cast<uint32_t>(m_virtualMultiTurnCount);
    uint16_t highWord = static_cast<uint16_t>((multiTurnValue >> 16) & 0xFFFF);
    uint16_t lowWord = static_cast<uint16_t>(multiTurnValue & 0xFFFF);
    setHoldingRegister(REG_VIRTUAL_MULTITURN_HIGH, highWord);
    setHoldingRegister(REG_VIRTUAL_MULTITURN_LOW, lowWord);
    
    // Update angular velocity (signed 16-bit)
    int16_t velocityRaw = static_cast<int16_t>(m_simulatedVelocityRpm);
    setHoldingRegister(REG_ANGULAR_VELOCITY, static_cast<uint16_t>(velocityRaw));
}

} // namespace Simulation
} // namespace Infrastructure
