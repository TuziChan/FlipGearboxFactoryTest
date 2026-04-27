#ifndef MOCKMOTORDEVICE_H
#define MOCKMOTORDEVICE_H

#include "MockModbusDevice.h"
#include <QVector>

namespace Infrastructure {
namespace Simulation {

/**
 * @brief Mock AQMD3610NS-A2 motor drive device
 * 
 * Register mapping:
 * - 0x0000: Device ID (read-only)
 * - 0x0011: Real-time current (×0.01A, read-only)
 * - 0x0040: Set speed (-1000 to 1000, ×0.1%)
 * - 0x0042: Stop and lock (write 1 to brake)
 * - 0x0044: Natural stop (write 1 to coast)
 * - 0x0050: AI1 port direction (0=input, 1=output)
 * - 0x0052: AI1 port level (0=low, 1=high)
 */
class MockMotorDevice : public MockModbusDevice {
    Q_OBJECT

public:
    explicit MockMotorDevice(uint8_t slaveId = 1, QObject* parent = nullptr);

    /**
     * @brief Set simulated motor current in Amperes
     */
    void setSimulatedCurrent(double currentA);

    /**
     * @brief Set AI1 input level (for magnet detection simulation)
     */
    void setAI1InputLevel(bool high);

    /**
     * @brief Get current motor speed setting (-1000 to 1000)
     */
    int16_t getMotorSpeed() const;

    /**
     * @brief Check if motor is braking
     */
    bool isBraking() const;

    /**
     * @brief Link encoder angle for automatic magnet detection
     */
    void linkEncoderAngle(double* anglePtr);

    /**
     * @brief Enable/disable automatic magnet detection
     */
    void setMagnetDetectionEnabled(bool enabled);

    /**
     * @brief Check if magnet detection is enabled
     */
    bool isMagnetDetectionEnabled() const;

    /**
     * @brief Set magnet positions (degrees)
     */
    void setMagnetPositions(const QVector<double>& positions);

    /**
     * @brief Get magnet positions
     */
    QVector<double> getMagnetPositions() const;

    /**
     * @brief Get magnet pass count for specific magnet
     */
    int getMagnetPassCount(int magnetIndex) const;

    /**
     * @brief Reset magnet detection state
     */
    void resetMagnetDetection();

    /**
     * @brief Set detection window (degrees around magnet position)
     */
    void setDetectionWindow(double windowDeg);

protected:
    void onRegisterWrite(uint16_t address, uint16_t value) override;
    void updateDynamicRegisters() override;

private:
    static constexpr uint16_t REG_DEVICE_ID = 0x0000;
    static constexpr uint16_t REG_REAL_TIME_CURRENT = 0x0011;
    static constexpr uint16_t REG_SET_SPEED = 0x0040;
    static constexpr uint16_t REG_STOP_AND_LOCK = 0x0042;
    static constexpr uint16_t REG_NATURAL_STOP = 0x0044;
    static constexpr uint16_t REG_AI1_PORT_DIRECTION = 0x0050;
    static constexpr uint16_t REG_AI1_PORT_LEVEL = 0x0052;

    double m_simulatedCurrentA;
    bool m_ai1InputLevel;
    bool m_isBraking;

    // Automatic magnet detection
    double* m_linkedEncoderAngle;
    bool m_magnetDetectionEnabled;
    QVector<double> m_magnetPositions;
    QVector<int> m_magnetPassCounts;
    QVector<bool> m_magnetLastState;
    double m_detectionWindowDeg;

    void updateMagnetDetection();
    bool isAngleInWindow(double angle, double targetAngle, double window) const;
};

} // namespace Simulation
} // namespace Infrastructure

#endif // MOCKMOTORDEVICE_H
