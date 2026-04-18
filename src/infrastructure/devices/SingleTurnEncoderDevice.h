#ifndef SINGLETURNENCODER_H
#define SINGLETURNENCODER_H

#include "IEncoderDevice.h"
#include "../bus/IBusController.h"

namespace Infrastructure {
namespace Devices {

/**
 * @brief Single-turn encoder device implementation
 * 
 * Supports four communication modes:
 * - Mode 0 (default): Query mode (Modbus RTU polling)
 * - Mode 2: Auto-report single-turn value
 * - Mode 3: Auto-report virtual multi-turn value
 * - Mode 4: Auto-report angular velocity
 * 
 * Register mapping per 2026-04-17-device-registers-correction.md
 * 
 * REG_ANGLE (0x0000): Raw count value (0 to resolution) - single turn
 * REG_VIRTUAL_MULTITURN (0x0000-0x0001): 32-bit accumulated count - multi-turn
 * REG_ANGULAR_VELOCITY (0x0003): Signed 16-bit velocity value
 * REG_AUTO_REPORT_MODE (0x0006): Auto-report mode (0x00=off, 0x01=single, 0x04=multi, 0x05=velocity)
 * REG_AUTO_REPORT_INTERVAL (0x0007): Auto-report interval in ms
 * REG_SET_ZERO (0x0008): Write 1 to set current position as zero
 * 
 * Angle conversion: angleDeg = (count / resolution) × 360°
 */
class SingleTurnEncoderDevice : public IEncoderDevice {
    Q_OBJECT

public:
    explicit SingleTurnEncoderDevice(Bus::IBusController* busController, 
                                      uint8_t slaveId,
                                      uint16_t resolution = 4096,
                                      int communicationMode = 0,
                                      int autoReportIntervalMs = 20,
                                      QObject* parent = nullptr);
    ~SingleTurnEncoderDevice() override = default;

    bool initialize() override;
    bool readAngle(double& angleDeg) override;
    bool readVirtualMultiTurn(double& totalAngleDeg) override;
    bool readAngularVelocity(double& velocityRpm) override;
    bool setZeroPoint() override;
    QString lastError() const override;

    /**
     * @brief Set auto-report mode at runtime
     * @param mode 0x00=off, 0x01=single-turn, 0x04=multi-turn, 0x05=velocity
     * @param intervalMs Report interval in milliseconds
     */
    bool setAutoReportMode(uint16_t mode, int intervalMs);

private:
    Bus::IBusController* m_busController;
    uint8_t m_slaveId;
    uint16_t m_resolution;
    int m_communicationMode;
    int m_autoReportIntervalMs;
    QString m_lastError;

    // Register addresses (from correction document)
    static constexpr uint16_t REG_ANGLE = 0x0000;                  // Raw count value (single-turn)
    static constexpr uint16_t REG_VIRTUAL_MULTITURN = 0x0000;      // 32-bit multi-turn count (2 registers)
    static constexpr uint16_t REG_ANGULAR_VELOCITY = 0x0003;       // Signed 16-bit velocity
    static constexpr uint16_t REG_AUTO_REPORT_MODE = 0x0006;       // Auto-report mode
    static constexpr uint16_t REG_AUTO_REPORT_INTERVAL = 0x0007;   // Auto-report interval (ms)
    static constexpr uint16_t REG_SET_ZERO = 0x0008;               // Write 1 to set zero

    bool readRegisters(uint16_t address, uint16_t count, QVector<uint16_t>& values);
    bool writeRegister(uint16_t address, uint16_t value);
};

} // namespace Devices
} // namespace Infrastructure

#endif // SINGLETURNENCODER_H
