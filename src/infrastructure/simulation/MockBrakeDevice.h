#ifndef MOCKBRAKEDEVICE_H
#define MOCKBRAKEDEVICE_H

#include "MockModbusDevice.h"

namespace Infrastructure {
namespace Simulation {

/**
 * @brief Mock dual-channel programmable power supply device
 * 
 * Register mapping:
 * Holding registers (0x03/0x06):
 * - 0x0000: CH1 set voltage (×0.01V)
 * - 0x0001: CH1 set current (×0.01A)
 * - 0x0002: CH2 set voltage (×0.01V)
 * - 0x0003: CH2 set current (×0.01A)
 * 
 * Input registers (0x04):
 * - 0x0000: CH1 actual voltage (×0.01V)
 * - 0x0001: CH1 actual current (×0.01A)
 * - 0x0002: CH1 actual power (×0.01W)
 * - 0x0004: CH2 actual voltage (×0.01V)
 * - 0x0005: CH2 actual current (×0.01A)
 * - 0x0006: CH2 actual power (×0.01W)
 * - 0x0009: Mode register (bit 0=CH1 mode, bit 1=CH2 mode, 0=CC, 1=CV)
 * 
 * Coils (0x01/0x05):
 * - 0x0000: CH1 output enable
 * - 0x0001: CH2 output enable
 */
class MockBrakeDevice : public MockModbusDevice {
    Q_OBJECT

public:
    explicit MockBrakeDevice(uint8_t slaveId = 4, QObject* parent = nullptr);

    /**
     * @brief Set channel mode (0=CC, 1=CV)
     */
    void setChannelMode(int channel, int mode);

protected:
    void onRegisterWrite(uint16_t address, uint16_t value) override;
    void onCoilWrite(uint16_t address, bool value) override;
    void updateDynamicRegisters() override;

private:
    static constexpr uint16_t REG_CH1_SET_VOLTAGE = 0x0000;
    static constexpr uint16_t REG_CH1_SET_CURRENT = 0x0001;
    static constexpr uint16_t REG_CH2_SET_VOLTAGE = 0x0002;
    static constexpr uint16_t REG_CH2_SET_CURRENT = 0x0003;
    
    static constexpr uint16_t REG_CH1_READ_VOLTAGE = 0x0000;
    static constexpr uint16_t REG_CH1_READ_CURRENT = 0x0001;
    static constexpr uint16_t REG_CH1_READ_POWER = 0x0002;
    static constexpr uint16_t REG_CH2_READ_VOLTAGE = 0x0004;
    static constexpr uint16_t REG_CH2_READ_CURRENT = 0x0005;
    static constexpr uint16_t REG_CH2_READ_POWER = 0x0006;
    static constexpr uint16_t REG_MODE = 0x0009;
    
    static constexpr uint16_t COIL_CH1_OUTPUT_ENABLE = 0x0000;
    static constexpr uint16_t COIL_CH2_OUTPUT_ENABLE = 0x0001;

    struct ChannelState {
        double setVoltageV = 0.0;
        double setCurrentA = 0.0;
        bool outputEnabled = false;
        int mode = 0; // 0=CC, 1=CV
    };

    ChannelState m_channel1;
    ChannelState m_channel2;

    void updateChannelOutput(int channel);
};

} // namespace Simulation
} // namespace Infrastructure

#endif // MOCKBRAKEDEVICE_H
