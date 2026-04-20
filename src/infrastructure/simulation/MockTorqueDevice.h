#ifndef MOCKTORQUEDEVICE_H
#define MOCKTORQUEDEVICE_H

#include "MockModbusDevice.h"

namespace Infrastructure {
namespace Simulation {

/**
 * @brief Mock DYN200 torque sensor device
 * 
 * Register mapping (32-bit big-endian values):
 * - 0x0000-0x0001: Torque (×0.01 N·m)
 * - 0x0002-0x0003: Speed (×1 RPM, no scaling)
 * - 0x0004-0x0005: Power (×0.1 W)
 * - 0x001C: Communication mode (0=Modbus RTU)
 */
class MockTorqueDevice : public MockModbusDevice {
    Q_OBJECT

public:
    explicit MockTorqueDevice(uint8_t slaveId = 2, QObject* parent = nullptr);

    /**
     * @brief Set simulated torque in N·m
     */
    void setSimulatedTorque(double torqueNm);

    /**
     * @brief Set simulated speed in RPM
     */
    void setSimulatedSpeed(double speedRpm);

    /**
     * @brief Set simulated power in Watts
     */
    void setSimulatedPower(double powerW);

protected:
    void updateDynamicRegisters() override;

private:
    static constexpr uint16_t REG_TORQUE_HIGH = 0x0000;
    static constexpr uint16_t REG_TORQUE_LOW = 0x0001;
    static constexpr uint16_t REG_SPEED_HIGH = 0x0002;
    static constexpr uint16_t REG_SPEED_LOW = 0x0003;
    static constexpr uint16_t REG_POWER_HIGH = 0x0004;
    static constexpr uint16_t REG_POWER_LOW = 0x0005;
    static constexpr uint16_t REG_COMM_MODE = 0x001C;

    double m_simulatedTorqueNm;
    double m_simulatedSpeedRpm;
    double m_simulatedPowerW;

    void set32BitRegister(uint16_t highAddr, uint16_t lowAddr, int32_t value);
};

} // namespace Simulation
} // namespace Infrastructure

#endif // MOCKTORQUEDEVICE_H
