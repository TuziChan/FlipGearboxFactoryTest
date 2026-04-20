#include "MockDeviceTestExample.h"
#include <QDebug>
#include <QThread>
#include <QCoreApplication>

namespace Infrastructure {
namespace Simulation {

MockDeviceTestExample::MockDeviceTestExample(QObject* parent)
    : QObject(parent)
    , m_mockManager(new MockDeviceManager(this))
    , m_motorDriver(nullptr)
    , m_torqueSensor(nullptr)
    , m_encoder(nullptr)
    , m_brake(nullptr)
    , m_testsPassed(0)
    , m_testsFailed(0)
{
    // Initialize mock devices
    m_mockManager->initializeDefaultDevices();
    
    // Setup device drivers
    setupDeviceDrivers();
}

void MockDeviceTestExample::setupDeviceDrivers() {
    auto* busController = m_mockManager->busController();

    // Create motor driver (COM1, slave 1, 9600 baud, Even parity)
    m_motorDriver = new Devices::AqmdMotorDriveDevice(busController, 1, this);

    // Create torque sensor (COM2, slave 2, 19200 baud, None parity, 2 stop bits)
    m_torqueSensor = new Devices::Dyn200TorqueSensorDevice(busController, 2, 0, this);

    // Create encoder (COM3, slave 3, 9600 baud, None parity, resolution 4096)
    m_encoder = new Devices::SingleTurnEncoderDevice(busController, 3, 4096, 0, 20, this);

    // Create brake (COM4, slave 4, 9600 baud, None parity)
    m_brake = new Devices::BrakePowerSupplyDevice(busController, 4, this);
}

void MockDeviceTestExample::runAllTests() {
    qDebug() << "=== Starting Mock Device Test Suite ===";
    
    m_testsPassed = 0;
    m_testsFailed = 0;

    testBasicCommunication();
    testAngleMagnetDetection();
    testHighSpeedAcquisition();
    testErrorInjection();
    testDataLinkBlocking();
    testBoundaryConditions();

    qDebug() << "=== Test Suite Completed ===";
    qDebug() << "Passed:" << m_testsPassed << "Failed:" << m_testsFailed;
    
    emit allTestsCompleted(m_testsPassed, m_testsFailed);
}

void MockDeviceTestExample::testBasicCommunication() {
    qDebug() << "\n--- Test 1: Basic Multi-Device Communication ---";

    // Open all ports
    bool motorOpen = m_mockManager->busController()->open("COM1", 9600, 1000, "Even", 1);
    logTest("Motor port open", motorOpen, motorOpen ? "Success" : "Failed to open COM1");

    if (!motorOpen) return;

    // Initialize motor
    bool motorInit = m_motorDriver->initialize();
    logTest("Motor initialize", motorInit, motorInit ? "Motor initialized" : m_motorDriver->lastError());

    // Test motor control
    bool motorSet = m_motorDriver->setMotor(Devices::IMotorDriveDevice::Direction::Forward, 50.0);
    logTest("Motor set speed", motorSet, motorSet ? "Motor speed set to 50%" : m_motorDriver->lastError());

    // Read motor current
    double currentA = 0.0;
    bool currentRead = m_motorDriver->readCurrent(currentA);
    logTest("Motor read current", currentRead, 
            currentRead ? QString("Current: %1 A").arg(currentA) : m_motorDriver->lastError());

    m_mockManager->busController()->close();

    // Test torque sensor
    bool torqueOpen = m_mockManager->busController()->open("COM2", 19200, 1000, "None", 2);
    logTest("Torque port open", torqueOpen, torqueOpen ? "Success" : "Failed to open COM2");

    if (torqueOpen) {
        m_mockManager->torqueDevice()->setSimulatedTorque(5.5);
        m_mockManager->torqueDevice()->setSimulatedSpeed(1500.0);

        double torqueNm = 0.0, speedRpm = 0.0, powerW = 0.0;
        bool torqueRead = m_torqueSensor->readAll(torqueNm, speedRpm, powerW);
        logTest("Torque read all", torqueRead,
                torqueRead ? QString("T=%1Nm, S=%2RPM, P=%3W").arg(torqueNm).arg(speedRpm).arg(powerW)
                           : m_torqueSensor->lastError());

        m_mockManager->busController()->close();
    }
}

void MockDeviceTestExample::testAngleMagnetDetection() {
    qDebug() << "\n--- Test 2: Angle Test with Magnet Detection ---";
    qDebug() << "Magnet positions: 3°, 49°, 113°";
    qDebug() << "Detection window: ±2°";

    bool encoderOpen = m_mockManager->busController()->open("COM3", 9600, 1000, "None", 1);
    bool motorOpen = m_mockManager->busController()->open("COM1", 9600, 1000, "Even", 1);

    if (!encoderOpen || !motorOpen) {
        logTest("Angle test setup", false, "Failed to open ports");
        return;
    }

    struct TestAngle {
        double angle;
        bool shouldDetect;
        QString description;
    };

    QVector<TestAngle> testAngles = {
        {0.0, false, "Before first magnet"},
        {3.0, true, "At first magnet (3°)"},
        {5.0, false, "After first magnet"},
        {47.0, false, "Before second magnet"},
        {49.0, true, "At second magnet (49°)"},
        {51.0, false, "After second magnet"},
        {111.0, false, "Before third magnet"},
        {113.0, true, "At third magnet (113°)"},
        {115.0, false, "After third magnet"},
        {180.0, false, "Halfway point"},
        {270.0, false, "Three-quarter point"},
        {359.0, false, "Near wrap-around"}
    };

    int magnetTestsPassed = 0;
    int magnetTestsFailed = 0;

    for (const auto& test : testAngles) {
        // Simulate angle scenario
        m_mockManager->simulateAngleTestScenario(test.angle);

        // Read encoder angle
        m_mockManager->busController()->open("COM3", 9600, 1000, "None", 1);
        double readAngle = 0.0;
        bool angleRead = m_encoder->readAngle(readAngle);

        // Read magnet detection
        m_mockManager->busController()->open("COM1", 9600, 1000, "Even", 1);
        bool magnetLevel = false;
        bool levelRead = m_motorDriver->readAI1Level(magnetLevel);

        bool testPassed = angleRead && levelRead && (magnetLevel == test.shouldDetect);
        
        QString result = QString("Angle=%1°, Magnet=%2, Expected=%3 - %4")
                            .arg(readAngle, 0, 'f', 1)
                            .arg(magnetLevel ? "YES" : "NO")
                            .arg(test.shouldDetect ? "YES" : "NO")
                            .arg(test.description);

        if (testPassed) {
            magnetTestsPassed++;
            qDebug() << "  [PASS]" << result;
        } else {
            magnetTestsFailed++;
            qDebug() << "  [FAIL]" << result;
        }
    }

    logTest("Angle magnet detection", magnetTestsFailed == 0,
            QString("%1/%2 tests passed").arg(magnetTestsPassed).arg(testAngles.size()));

    m_mockManager->busController()->close();
}

void MockDeviceTestExample::testHighSpeedAcquisition() {
    qDebug() << "\n--- Test 3: High-Speed Concurrent Data Acquisition ---";

    bool success = true;
    int acquisitionCount = 100;
    int failedReads = 0;

    for (int i = 0; i < acquisitionCount; ++i) {
        // Simulate rotating system
        double angle = (i * 360.0) / acquisitionCount;
        m_mockManager->simulateAngleTestScenario(angle);
        m_mockManager->simulateMotorWithLoad(50.0, 5.0);

        // Read encoder
        m_mockManager->busController()->open("COM3", 9600, 1000, "None", 1);
        double readAngle = 0.0;
        if (!m_encoder->readAngle(readAngle)) {
            failedReads++;
        }
        m_mockManager->busController()->close();

        // Read torque
        m_mockManager->busController()->open("COM2", 19200, 1000, "None", 2);
        double torque = 0.0;
        if (!m_torqueSensor->readTorque(torque)) {
            failedReads++;
        }
        m_mockManager->busController()->close();

        // Small delay to simulate real-time acquisition
        QThread::msleep(10);
    }

    success = (failedReads == 0);
    logTest("High-speed acquisition", success,
            QString("%1 acquisitions, %2 failed reads").arg(acquisitionCount).arg(failedReads));
}

void MockDeviceTestExample::testErrorInjection() {
    qDebug() << "\n--- Test 4: Error Injection and Recovery ---";

    // Enable error injection
    m_mockManager->enableErrorInjection(0.1, 0.05, 0.1); // 10% CRC, 5% timeout, 10% delay

    int totalAttempts = 50;
    int successfulReads = 0;
    int crcErrors = 0;
    int timeouts = 0;

    for (int i = 0; i < totalAttempts; ++i) {
        m_mockManager->busController()->open("COM3", 9600, 1000, "None", 1);
        double angle = 0.0;
        bool success = m_encoder->readAngle(angle);
        
        if (success) {
            successfulReads++;
        } else {
            QString error = m_encoder->lastError();
            if (error.contains("CRC")) {
                crcErrors++;
            } else if (error.contains("timeout")) {
                timeouts++;
            }
        }
        
        m_mockManager->busController()->close();
        QThread::msleep(20);
    }

    // Disable error injection
    m_mockManager->disableErrorInjection();

    QString result = QString("Success=%1, CRC errors=%2, Timeouts=%3 out of %4 attempts")
                        .arg(successfulReads).arg(crcErrors).arg(timeouts).arg(totalAttempts);
    
    bool testPassed = (successfulReads > totalAttempts * 0.7); // At least 70% success
    logTest("Error injection", testPassed, result);
}

void MockDeviceTestExample::testDataLinkBlocking() {
    qDebug() << "\n--- Test 5: Data Link Blocking Scenarios ---";

    // Test concurrent access to multiple devices
    bool success = true;
    int iterations = 20;

    for (int i = 0; i < iterations; ++i) {
        // Rapidly switch between devices
        m_mockManager->busController()->open("COM1", 9600, 500, "Even", 1);
        double current = 0.0;
        bool r1 = m_motorDriver->readCurrent(current);
        m_mockManager->busController()->close();

        m_mockManager->busController()->open("COM2", 19200, 500, "None", 2);
        double torque = 0.0;
        bool r2 = m_torqueSensor->readTorque(torque);
        m_mockManager->busController()->close();

        m_mockManager->busController()->open("COM3", 9600, 500, "None", 1);
        double angle = 0.0;
        bool r3 = m_encoder->readAngle(angle);
        m_mockManager->busController()->close();

        if (!r1 || !r2 || !r3) {
            success = false;
        }
    }

    logTest("Data link blocking", success,
            QString("%1 iterations of rapid device switching").arg(iterations));
}

void MockDeviceTestExample::testBoundaryConditions() {
    qDebug() << "\n--- Test 6: Boundary and Edge Cases ---";

    int subTestsPassed = 0;
    int subTestsTotal = 0;

    // Test 1: Zero angle
    subTestsTotal++;
    m_mockManager->encoderDevice()->setSimulatedAngle(0.0);
    m_mockManager->busController()->open("COM3", 9600, 1000, "None", 1);
    double angle = 0.0;
    if (m_encoder->readAngle(angle) && qAbs(angle) < 1.0) {
        subTestsPassed++;
        qDebug() << "  [PASS] Zero angle test";
    } else {
        qDebug() << "  [FAIL] Zero angle test";
    }
    m_mockManager->busController()->close();

    // Test 2: 360° wraparound
    subTestsTotal++;
    m_mockManager->encoderDevice()->setSimulatedAngle(359.5);
    m_mockManager->busController()->open("COM3", 9600, 1000, "None", 1);
    if (m_encoder->readAngle(angle) && angle >= 359.0 && angle < 360.0) {
        subTestsPassed++;
        qDebug() << "  [PASS] 360° wraparound test";
    } else {
        qDebug() << "  [FAIL] 360° wraparound test";
    }
    m_mockManager->busController()->close();

    // Test 3: Negative torque
    subTestsTotal++;
    m_mockManager->torqueDevice()->setSimulatedTorque(-5.0);
    m_mockManager->busController()->open("COM2", 19200, 1000, "None", 2);
    double torque = 0.0;
    if (m_torqueSensor->readTorque(torque) && torque < 0.0) {
        subTestsPassed++;
        qDebug() << "  [PASS] Negative torque test";
    } else {
        qDebug() << "  [FAIL] Negative torque test";
    }
    m_mockManager->busController()->close();

    // Test 4: Maximum current
    subTestsTotal++;
    m_mockManager->motorDevice()->setSimulatedCurrent(10.0);
    m_mockManager->busController()->open("COM1", 9600, 1000, "Even", 1);
    double current = 0.0;
    if (m_motorDriver->readCurrent(current) && current >= 9.5 && current <= 10.5) {
        subTestsPassed++;
        qDebug() << "  [PASS] Maximum current test";
    } else {
        qDebug() << "  [FAIL] Maximum current test";
    }
    m_mockManager->busController()->close();

    logTest("Boundary conditions", subTestsPassed == subTestsTotal,
            QString("%1/%2 sub-tests passed").arg(subTestsPassed).arg(subTestsTotal));
}

void MockDeviceTestExample::logTest(const QString& testName, bool success, const QString& message) {
    if (success) {
        m_testsPassed++;
        qDebug() << "[PASS]" << testName << "-" << message;
    } else {
        m_testsFailed++;
        qDebug() << "[FAIL]" << testName << "-" << message;
    }
    
    emit testCompleted(testName, success, message);
}

} // namespace Simulation
} // namespace Infrastructure
