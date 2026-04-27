#include <QCoreApplication>
#include <QDebug>
#include "src/infrastructure/simulation/MockDeviceManager.h"
#include "src/infrastructure/devices/AqmdMotorDriveDevice.h"
#include "src/infrastructure/devices/Dyn200TorqueSensorDevice.h"
#include "src/infrastructure/devices/SingleTurnEncoderDevice.h"
#include "src/infrastructure/devices/BrakePowerSupplyDevice.h"

using namespace Infrastructure::Simulation;
using namespace Infrastructure::Devices;

void testBasicCommunication(MockDeviceManager* manager);
void testAngleMagnetDetection(MockDeviceManager* manager);
void testErrorInjection(MockDeviceManager* manager);

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "=== Mock Device Simulator Quick Test ===\n";

    // Create manager and initialize devices
    auto* manager = new MockDeviceManager(&app);
    manager->initializeDefaultDevices();

    // Run tests
    testBasicCommunication(manager);
    testAngleMagnetDetection(manager);
    testErrorInjection(manager);

    qDebug() << "\n=== All Tests Completed ===";

    return 0;
}

void testBasicCommunication(MockDeviceManager* manager)
{
    qDebug() << "--- Test 1: Basic Communication ---";

    auto* busController = manager->busController();
    auto* motorDriver = new AqmdMotorDriveDevice(busController, 1);
    auto* torqueSensor = new Dyn200TorqueSensorDevice(busController, 2);
    auto* encoder = new SingleTurnEncoderDevice(busController, 3, 4096);
    auto* brake = new BrakePowerSupplyDevice(busController, 4);

    // Test motor
    busController->open("COM1", 9600, 1000, "Even", 1);
    if (motorDriver->initialize()) {
        qDebug() << "[PASS] Motor initialized";
        
        manager->motorDevice()->setSimulatedCurrent(2.5);
        double current = 0.0;
        if (motorDriver->readCurrent(current)) {
            qDebug() << "[PASS] Motor current read:" << current << "A";
        }
    }
    busController->close();

    // Test torque sensor
    busController->open("COM2", 19200, 1000, "None", 2);
    manager->torqueDevice()->setSimulatedTorque(5.5);
    manager->torqueDevice()->setSimulatedSpeed(1500.0);
    
    double torque = 0.0, speed = 0.0, power = 0.0;
    if (torqueSensor->readAll(torque, speed, power)) {
        qDebug() << "[PASS] Torque sensor read: T=" << torque << "Nm, S=" << speed << "RPM, P=" << power << "W";
    }
    busController->close();

    // Test encoder
    busController->open("COM3", 9600, 1000, "None", 1);
    manager->encoderDevice()->setSimulatedAngle(45.0);
    
    double angle = 0.0;
    if (encoder->readAngle(angle)) {
        qDebug() << "[PASS] Encoder read:" << angle << "degrees";
    }
    busController->close();

    // Test brake
    busController->open("COM4", 9600, 1000, "None", 1);
    if (brake->initialize()) {
        qDebug() << "[PASS] Brake initialized";
        
        if (brake->setCurrent(1, 2.0)) {
            qDebug() << "[PASS] Brake current set to 2.0A";
        }
    }
    busController->close();

    delete motorDriver;
    delete torqueSensor;
    delete encoder;
    delete brake;
}

void testAngleMagnetDetection(MockDeviceManager* manager)
{
    qDebug() << "\n--- Test 2: Angle Magnet Detection ---";
    qDebug() << "Magnet positions: 3°, 49°, 113.5° (±2° detection window)";

    auto* busController = manager->busController();
    auto* motorDriver = new AqmdMotorDriveDevice(busController, 1);
    auto* encoder = new SingleTurnEncoderDevice(busController, 3, 4096);

    struct TestCase {
        double angle;
        bool shouldDetect;
        QString description;
    };

    QVector<TestCase> testCases = {
        {3.0, true, "At first magnet (3°)"},
        {5.0, false, "After first magnet"},
        {49.0, true, "At second magnet (49°)"},
        {51.0, false, "After second magnet"},
        {113.5, true, "At third magnet (113.5°)"},
        {115.0, false, "After third magnet"},
        {180.0, false, "No magnet at 180°"}
    };

    int passed = 0;
    for (const auto& test : testCases) {
        manager->simulateAngleTestScenario(test.angle);

        busController->open("COM3", 9600, 1000, "None", 1);
        double readAngle = 0.0;
        encoder->readAngle(readAngle);
        busController->close();

        busController->open("COM1", 9600, 1000, "Even", 1);
        bool magnetLevel = false;
        motorDriver->readAI1Level(magnetLevel);
        busController->close();

        bool testPassed = (magnetLevel == test.shouldDetect);
        if (testPassed) {
            passed++;
            qDebug() << "[PASS]" << test.description << "- Magnet:" << (magnetLevel ? "YES" : "NO");
        } else {
            qDebug() << "[FAIL]" << test.description << "- Expected:" << test.shouldDetect << "Got:" << magnetLevel;
        }
    }

    qDebug() << "Magnet detection tests:" << passed << "/" << testCases.size() << "passed";

    delete motorDriver;
    delete encoder;
}

void testErrorInjection(MockDeviceManager* manager)
{
    qDebug() << "\n--- Test 3: Error Injection ---";

    auto* busController = manager->busController();
    auto* encoder = new SingleTurnEncoderDevice(busController, 3, 4096);

    // Enable error injection
    manager->enableErrorInjection(0.2, 0.1, 0.1);  // 20% CRC, 10% timeout, 10% delay

    int totalAttempts = 20;
    int successCount = 0;
    int crcErrors = 0;
    int timeouts = 0;

    for (int i = 0; i < totalAttempts; ++i) {
        busController->open("COM3", 9600, 1000, "None", 1);
        double angle = 0.0;
        bool success = encoder->readAngle(angle);
        
        if (success) {
            successCount++;
        } else {
            QString error = encoder->lastError();
            if (error.contains("CRC")) {
                crcErrors++;
            } else if (error.contains("timeout")) {
                timeouts++;
            }
        }
        busController->close();
    }

    manager->disableErrorInjection();

    qDebug() << "[INFO] Error injection results:";
    qDebug() << "  Success:" << successCount << "/" << totalAttempts;
    qDebug() << "  CRC errors:" << crcErrors;
    qDebug() << "  Timeouts:" << timeouts;

    if (successCount > totalAttempts * 0.5) {
        qDebug() << "[PASS] Error injection test (>50% success rate)";
    } else {
        qDebug() << "[FAIL] Error injection test (too many errors)";
    }

    delete encoder;
}
