#ifndef BRAKEPOWERSUPPLYDEVICE_H
#define BRAKEPOWERSUPPLYDEVICE_H

#include "IBrakePowerDevice.h"
#include "../bus/IBusController.h"

namespace Infrastructure {
namespace Devices {

/**
 * @brief Dual-channel programmable power supply device implementation
 * 
 * Register mapping per 2026-04-17-device-registers-correction.md
 * 
 * Holding Registers (0x03/0x06): Voltage/Current setpoints
 * Input Registers (0x04): Actual voltage/current/power readback
 * Coils (0x01/0x05): Output enable/disable
 * 
 */
class BrakePowerSupplyDevice : public IBrakePowerDevice {
    Q_OBJECT

public:
    explicit BrakePowerSupplyDevice(Bus::IBusController* busController, 
                                     uint8_t slaveId,
                                     QObject* parent = nullptr);
    ~BrakePowerSupplyDevice() override = default;

    bool initialize() override;
    bool setCurrent(int channel, double currentA) override;
    bool setOutputEnable(int channel, bool enable) override;
    bool readCurrent(int channel, double& currentA) override;
    QString lastError() const override;

private:
    Bus::IBusController* m_busController;
    uint8_t m_slaveId;
    QString m_lastError;

    // Register addresses (from correction document)
    // Holding registers for setpoints
    static constexpr uint16_t REG_CH1_SET_CURRENT = 0x0001;  // ×0.01A
    static constexpr uint16_t REG_CH2_SET_CURRENT = 0x0003;  // ×0.01A
    
    // Input registers for readback (would use 0x04 function code)
    static constexpr uint16_t REG_CH1_READ_CURRENT = 0x0001;  // ×0.01A
    static constexpr uint16_t REG_CH2_READ_CURRENT = 0x0004;  // ×0.01A
    
    // Coils for output enable (would use 0x01/0x05 function codes)
    static constexpr uint16_t COIL_CH1_OUTPUT_ENABLE = 0x0000;
    static constexpr uint16_t COIL_CH2_OUTPUT_ENABLE = 0x0001;

    bool readHoldingRegisters(uint16_t address, uint16_t count, QVector<uint16_t>& values);
    bool readInputRegisters(uint16_t address, uint16_t count, QVector<uint16_t>& values);
    bool writeRegister(uint16_t address, uint16_t value);
    bool writeCoil(uint16_t address, bool value);
    uint16_t getSetCurrentRegister(int channel) const;
    uint16_t getOutputEnableCoil(int channel) const;
    uint16_t getReadCurrentRegister(int channel) const;
};

} // namespace Devices
} // namespace Infrastructure

#endif // BRAKEPOWERSUPPLYDEVICE_H
