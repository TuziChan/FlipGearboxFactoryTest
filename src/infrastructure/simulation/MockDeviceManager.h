#ifndef MOCKDEVICEMANAGER_H
#define MOCKDEVICEMANAGER_H

#include "MockSerialBusController.h"
#include "MockMotorDevice.h"
#include "MockTorqueDevice.h"
#include "MockEncoderDevice.h"
#include "MockBrakeDevice.h"
#include <QObject>

namespace Infrastructure {
namespace Simulation {

/**
 * @brief Manager for multi-device mock simulation
 * 
 * Provides high-level API for setting up and controlling mock devices
 * for comprehensive testing scenarios.
 */
class MockDeviceManager : public QObject {
    Q_OBJECT

public:
    explicit MockDeviceManager(QObject* parent = nullptr);
    ~MockDeviceManager() override = default;

    /**
     * @brief Initialize all devices with default configuration
     * 
     * Creates:
     * - Motor on COM1 (slave 1, 9600 baud, Even parity)
     * - Torque sensor on COM2 (slave 2, 19200 baud, None parity, 2 stop bits)
     * - Encoder on COM3 (slave 3, 9600 baud, None parity)
     * - Brake on COM4 (slave 4, 9600 baud, None parity)
     */
    void initializeDefaultDevices();

    /**
     * @brief Get the mock bus controller
     */
    MockSerialBusController* busController() { return m_busController; }

    /**
     * @brief Get motor device
     */
    MockMotorDevice* motorDevice() const { return m_motorDevice; }

    /**
     * @brief Get torque sensor device
     */
    MockTorqueDevice* torqueDevice() const { return m_torqueDevice; }

    /**
     * @brief Get encoder device
     */
    MockEncoderDevice* encoderDevice() const { return m_encoderDevice; }

    /**
     * @brief Get brake device
     */
    MockBrakeDevice* brakeDevice() const { return m_brakeDevice; }

    /**
     * @brief Simulate angle test scenario with magnet detection
     * @param angleDeg Current angle in degrees
     * 
     * Magnet positions: 3°, 49°, 113.5°
     * Detection window: ±2° around each magnet
     */
    void simulateAngleTestScenario(double angleDeg);

    /**
     * @brief Simulate motor running with load
     * @param speedPercent Motor speed percentage (-100 to 100)
     * @param loadTorqueNm Load torque in N·m
     */
    void simulateMotorWithLoad(double speedPercent, double loadTorqueNm);

    /**
     * @brief Enable error injection for stress testing
     * @param crcErrorRate CRC error rate (0.0-1.0)
     * @param timeoutRate Timeout rate (0.0-1.0)
     * @param delayRate Delay rate (0.0-1.0)
     */
    void enableErrorInjection(double crcErrorRate, double timeoutRate, double delayRate);

    /**
     * @brief Disable all error injection
     */
    void disableErrorInjection();

    /**
     * @brief Simulate high-speed data acquisition scenario
     * @param durationMs Duration in milliseconds
     * @param updateRateHz Update rate in Hz
     */
    void simulateHighSpeedAcquisition(int durationMs, int updateRateHz);

    /**
     * @brief Reset all devices to initial state
     */
    void resetAllDevices();

private:
    MockSerialBusController* m_busController;
    MockMotorDevice* m_motorDevice;
    MockTorqueDevice* m_torqueDevice;
    MockEncoderDevice* m_encoderDevice;
    MockBrakeDevice* m_brakeDevice;

    // Magnet positions for angle test (degrees)
    static constexpr double MAGNET_POS_1 = 3.0;
    static constexpr double MAGNET_POS_2 = 49.0;
    static constexpr double MAGNET_POS_3 = 113.5;
    static constexpr double MAGNET_DETECTION_WINDOW = 2.0;

    bool isNearMagnet(double angleDeg, double magnetPos) const;
};

} // namespace Simulation
} // namespace Infrastructure

#endif // MOCKDEVICEMANAGER_H
