#ifndef MOCKENCODERDEVICE_H
#define MOCKENCODERDEVICE_H

#include "MockModbusDevice.h"

namespace Infrastructure {
namespace Simulation {

/**
 * @brief Mock single-turn encoder device
 * 
 * Register mapping:
 * - 0x0000: Single-turn raw count (0 to resolution)
 * - 0x0000-0x0001: Virtual multi-turn count (32-bit)
 * - 0x0003: Angular velocity (signed 16-bit RPM)
 * - 0x0006: Auto-report mode
 * - 0x0007: Auto-report interval (ms)
 * - 0x0008: Set zero point (write 1)
 * 
 * Angle conversion: angleDeg = (count / resolution) × 360°
 */
class MockEncoderDevice : public MockModbusDevice {
    Q_OBJECT

public:
    explicit MockEncoderDevice(uint8_t slaveId = 3, uint16_t resolution = 4096, QObject* parent = nullptr);

    /**
     * @brief Set simulated angle in degrees (0-360)
     */
    void setSimulatedAngle(double angleDeg);

    /**
     * @brief Set simulated angular velocity in RPM
     */
    void setSimulatedVelocity(double velocityRpm);

    /**
     * @brief Get current zero offset
     */
    uint16_t getZeroOffset() const { return m_zeroOffset; }

protected:
    void onRegisterWrite(uint16_t address, uint16_t value) override;
    void updateDynamicRegisters() override;

private:
    static constexpr uint16_t REG_ANGLE = 0x0000;
    static constexpr uint16_t REG_VIRTUAL_MULTITURN_HIGH = 0x0000;
    static constexpr uint16_t REG_VIRTUAL_MULTITURN_LOW = 0x0001;
    static constexpr uint16_t REG_ANGULAR_VELOCITY = 0x0003;
    static constexpr uint16_t REG_AUTO_REPORT_MODE = 0x0006;
    static constexpr uint16_t REG_AUTO_REPORT_INTERVAL = 0x0007;
    static constexpr uint16_t REG_SET_ZERO = 0x0008;

    uint16_t m_resolution;
    double m_simulatedAngleDeg;
    double m_simulatedVelocityRpm;
    uint16_t m_zeroOffset;
    int32_t m_virtualMultiTurnCount;
};

} // namespace Simulation
} // namespace Infrastructure

#endif // MOCKENCODERDEVICE_H
