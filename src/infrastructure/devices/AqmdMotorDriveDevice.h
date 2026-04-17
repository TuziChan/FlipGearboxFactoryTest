#ifndef AQMDMOTORDRIVEDEVICE_H
#define AQMDMOTORDRIVEDEVICE_H

#include "IMotorDriveDevice.h"
#include "../bus/IBusController.h"

namespace Infrastructure {
namespace Devices {

/**
 * @brief AQMD3610NS-A2 motor drive device implementation
 * 
 * Register mapping based on device manual in repository.
 */
class AqmdMotorDriveDevice : public IMotorDriveDevice {
    Q_OBJECT

public:
    explicit AqmdMotorDriveDevice(Bus::IBusController* busController, 
                                   uint8_t slaveId,
                                   QObject* parent = nullptr);
    ~AqmdMotorDriveDevice() override = default;

    bool initialize() override;
    bool setMotor(Direction direction, double dutyCyclePercent) override;
    bool brake() override;
    bool coast() override;
    bool readCurrent(double& currentA) override;
    bool readAI1Level(bool& level) override;
    QString lastError() const override;

private:
    Bus::IBusController* m_busController;
    uint8_t m_slaveId;
    QString m_lastError;

    // Register addresses (from device manual)
    static constexpr uint16_t REG_DEVICE_ID = 0x0000;
    static constexpr uint16_t REG_REAL_TIME_CURRENT = 0x0011;
    static constexpr uint16_t REG_SET_SPEED = 0x0040;
    static constexpr uint16_t REG_STOP_AND_LOCK = 0x0042;
    static constexpr uint16_t REG_NATURAL_STOP = 0x0044;
    static constexpr uint16_t REG_AI1_PORT_DIRECTION = 0x0050;
    static constexpr uint16_t REG_AI1_PORT_LEVEL = 0x0052;

    bool writeRegister(uint16_t address, uint16_t value);
    bool writeRegisterSigned(uint16_t address, int16_t value);
    bool readRegisters(uint16_t address, uint16_t count, QVector<uint16_t>& values);
};

} // namespace Devices
} // namespace Infrastructure

#endif // AQMDMOTORDRIVEDEVICE_H
