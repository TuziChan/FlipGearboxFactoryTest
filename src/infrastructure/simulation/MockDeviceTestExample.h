#ifndef MOCKDEVICETESTEXAMPLE_H
#define MOCKDEVICETESTEXAMPLE_H

#include "MockDeviceManager.h"
#include "../devices/AqmdMotorDriveDevice.h"
#include "../devices/Dyn200TorqueSensorDevice.h"
#include "../devices/SingleTurnEncoderDevice.h"
#include "../devices/BrakePowerSupplyDevice.h"
#include <QObject>
#include <QTimer>

namespace Infrastructure {
namespace Simulation {

/**
 * @brief Example test scenarios using mock devices
 * 
 * Demonstrates:
 * - Multi-port concurrent communication
 * - Angle test with magnet detection (3°, 49°, 113°)
 * - High-speed data acquisition
 * - Error injection and recovery
 * - Data link blocking scenarios
 */
class MockDeviceTestExample : public QObject {
    Q_OBJECT

public:
    explicit MockDeviceTestExample(QObject* parent = nullptr);
    ~MockDeviceTestExample() override = default;

    /**
     * @brief Run all test scenarios
     */
    void runAllTests();

    /**
     * @brief Test 1: Basic multi-device communication
     */
    void testBasicCommunication();

    /**
     * @brief Test 2: Angle test with magnet detection
     * 
     * Tests magnet detection at 3°, 49°, 113° with proper consideration
     * of previous magnet positions.
     */
    void testAngleMagnetDetection();

    /**
     * @brief Test 3: High-speed concurrent data acquisition
     * 
     * Tests multiple devices at high update rates (100+ Hz)
     */
    void testHighSpeedAcquisition();

    /**
     * @brief Test 4: Error injection and recovery
     * 
     * Tests CRC errors, timeouts, and delayed responses
     */
    void testErrorInjection();

    /**
     * @brief Test 5: Data link blocking scenarios
     * 
     * Tests concurrent requests and queue management
     */
    void testDataLinkBlocking();

    /**
     * @brief Test 6: Boundary and edge cases
     * 
     * Tests extreme values, zero crossings, and wraparound
     */
    void testBoundaryConditions();

signals:
    void testCompleted(const QString& testName, bool success, const QString& message);
    void allTestsCompleted(int passed, int failed);

private:
    MockDeviceManager* m_mockManager;
    Devices::AqmdMotorDriveDevice* m_motorDriver;
    Devices::Dyn200TorqueSensorDevice* m_torqueSensor;
    Devices::SingleTurnEncoderDevice* m_encoder;
    Devices::BrakePowerSupplyDevice* m_brake;

    int m_testsPassed;
    int m_testsFailed;

    void setupDeviceDrivers();
    void logTest(const QString& testName, bool success, const QString& message);
    bool verifyMotorCurrent(double expectedA, double toleranceA = 0.5);
    bool verifyEncoderAngle(double expectedDeg, double toleranceDeg = 1.0);
    bool verifyTorque(double expectedNm, double toleranceNm = 0.5);
};

} // namespace Simulation
} // namespace Infrastructure

#endif // MOCKDEVICETESTEXAMPLE_H
