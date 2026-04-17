#ifndef DYN200TORQUESENSORDEVICE_H
#define DYN200TORQUESENSORDEVICE_H

#include "ITorqueSensorDevice.h"
#include "../bus/IBusController.h"

namespace Infrastructure {
namespace Devices {

/**
 * @brief DYN200 torque sensor device implementation
 * 
 * Register mapping per 2026-04-17-device-registers-correction.md
 * All values are 32-bit signed integers (2 registers, big-endian).
 * 
 * Scaling:
 * - Torque: ×0.01 N·m
 * - Speed: ×1 RPM (NO scaling)
 * - Power: ×0.1 W
 */
class Dyn200TorqueSensorDevice : public ITorqueSensorDevice {
    Q_OBJECT

public:
    explicit Dyn200TorqueSensorDevice(Bus::IBusController* busController, 
                                       uint8_t slaveId,
                                       QObject* parent = nullptr);
    ~Dyn200TorqueSensorDevice() override = default;

    bool initialize() override;
    bool readTorque(double& torqueNm) override;
    bool readSpeed(double& speedRpm) override;
    bool readPower(double& powerW) override;
    bool readAll(double& torqueNm, double& speedRpm, double& powerW) override;
    QString lastError() const override;

private:
    Bus::IBusController* m_busController;
    uint8_t m_slaveId;
    QString m_lastError;

    // Register addresses (from correction document)
    static constexpr uint16_t REG_TORQUE = 0x0000;  // 2 registers, ×0.01 N·m
    static constexpr uint16_t REG_SPEED = 0x0002;   // 2 registers, ×1 RPM
    static constexpr uint16_t REG_POWER = 0x0004;   // 2 registers, ×0.1 W

    bool readRegisters(uint16_t address, uint16_t count, QVector<uint16_t>& values);
    int32_t combineToInt32(uint16_t highWord, uint16_t lowWord) const;
};

} // namespace Devices
} // namespace Infrastructure

#endif // DYN200TORQUESENSORDEVICE_H
