#ifndef SINGLETURNENCODER_H
#define SINGLETURNENCODER_H

#include "IEncoderDevice.h"
#include "../bus/IBusController.h"

namespace Infrastructure {
namespace Devices {

/**
 * @brief Single-turn encoder device implementation
 * 
 * Register mapping per 2026-04-17-device-registers-correction.md
 * 
 * REG_ANGLE (0x0000): Raw count value (0 to resolution)
 * Angle conversion: angleDeg = (count / resolution) × 360°
 * 
 * REG_SET_ZERO (0x0008): Write 1 to set current position as zero
 */
class SingleTurnEncoderDevice : public IEncoderDevice {
    Q_OBJECT

public:
    explicit SingleTurnEncoderDevice(Bus::IBusController* busController, 
                                      uint8_t slaveId,
                                      uint16_t resolution = 4096,
                                      QObject* parent = nullptr);
    ~SingleTurnEncoderDevice() override = default;

    bool initialize() override;
    bool readAngle(double& angleDeg) override;
    bool setZeroPoint() override;
    QString lastError() const override;

private:
    Bus::IBusController* m_busController;
    uint8_t m_slaveId;
    uint16_t m_resolution;
    QString m_lastError;

    // Register addresses (from correction document)
    static constexpr uint16_t REG_ANGLE = 0x0000;      // Raw count value
    static constexpr uint16_t REG_SET_ZERO = 0x0008;   // Write 1 to set zero

    bool readRegisters(uint16_t address, uint16_t count, QVector<uint16_t>& values);
    bool writeRegister(uint16_t address, uint16_t value);
};

} // namespace Devices
} // namespace Infrastructure

#endif // SINGLETURNENCODER_H
