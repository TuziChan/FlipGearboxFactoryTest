#include "MockDeviceManager.h"
#include <QThread>
#include <QtMath>

namespace Infrastructure {
namespace Simulation {

MockDeviceManager::MockDeviceManager(QObject* parent)
    : QObject(parent)
    , m_busController(new MockSerialBusController(this))
    , m_motorDevice(nullptr)
    , m_torqueDevice(nullptr)
    , m_encoderDevice(nullptr)
    , m_brakeDevice(nullptr)
{
}

void MockDeviceManager::initializeDefaultDevices() {
    // Create motor device (COM1, slave 1, 9600 baud, Even parity)
    m_motorDevice = new MockMotorDevice(1);
    m_busController->registerDevice("COM1", m_motorDevice);

    // Create torque sensor (COM2, slave 2, 19200 baud, None parity, 2 stop bits)
    m_torqueDevice = new MockTorqueDevice(2);
    m_busController->registerDevice("COM2", m_torqueDevice);

    // Create encoder (COM3, slave 3, 9600 baud, None parity, resolution 4096)
    m_encoderDevice = new MockEncoderDevice(3, 4096);
    m_busController->registerDevice("COM3", m_encoderDevice);

    // Create brake (COM4, slave 4, 9600 baud, None parity)
    m_brakeDevice = new MockBrakeDevice(4);
    m_busController->registerDevice("COM4", m_brakeDevice);
}

void MockDeviceManager::simulateAngleTestScenario(double angleDeg) {
    if (!m_encoderDevice || !m_motorDevice) {
        return;
    }

    // Update encoder angle
    m_encoderDevice->setSimulatedAngle(angleDeg);

    // Check if near any magnet position
    bool magnetDetected = isNearMagnet(angleDeg, MAGNET_POS_1) ||
                          isNearMagnet(angleDeg, MAGNET_POS_2) ||
                          isNearMagnet(angleDeg, MAGNET_POS_3);

    // Update motor AI1 input (magnet detection)
    m_motorDevice->setAI1InputLevel(magnetDetected);
}

void MockDeviceManager::simulateMotorWithLoad(double speedPercent, double loadTorqueNm) {
    if (!m_motorDevice || !m_torqueDevice) {
        return;
    }

    // Clamp speed to valid range
    speedPercent = qBound(-100.0, speedPercent, 100.0);

    // Calculate motor current based on speed and load
    double currentA = qAbs(speedPercent) * 0.05; // Base current
    currentA += qAbs(loadTorqueNm) * 0.5; // Additional current from load
    currentA = qMin(currentA, 10.0); // Cap at 10A

    m_motorDevice->setSimulatedCurrent(currentA);

    // Update torque sensor
    m_torqueDevice->setSimulatedTorque(loadTorqueNm);

    // Calculate speed in RPM (assuming max speed at 100% is 3000 RPM)
    double speedRpm = speedPercent * 30.0;
    m_torqueDevice->setSimulatedSpeed(speedRpm);

    // Calculate power: P = T × ω, where ω = 2π × n / 60
    double powerW = qAbs(loadTorqueNm) * qAbs(speedRpm) * 2.0 * M_PI / 60.0;
    m_torqueDevice->setSimulatedPower(powerW);
}

void MockDeviceManager::enableErrorInjection(double crcErrorRate, double timeoutRate, double delayRate) {
    MockModbusDevice::ErrorInjectionConfig config;
    config.enabled = true;
    config.crcErrorRate = qBound(0.0, crcErrorRate, 1.0);
    config.timeoutRate = qBound(0.0, timeoutRate, 1.0);
    config.delayRate = qBound(0.0, delayRate, 1.0);
    config.minDelayMs = 100;
    config.maxDelayMs = 500;
    config.exceptionRate = 0.0;

    m_busController->setGlobalErrorInjection(config);
}

void MockDeviceManager::disableErrorInjection() {
    MockModbusDevice::ErrorInjectionConfig config;
    config.enabled = false;
    m_busController->setGlobalErrorInjection(config);
}

void MockDeviceManager::simulateHighSpeedAcquisition(int durationMs, int updateRateHz) {
    if (!m_encoderDevice || !m_torqueDevice) {
        return;
    }

    int intervalMs = 1000 / updateRateHz;
    int iterations = durationMs / intervalMs;

    for (int i = 0; i < iterations; ++i) {
        // Simulate rotating angle
        double angleDeg = (i * 360.0 / iterations);
        m_encoderDevice->setSimulatedAngle(angleDeg);

        // Simulate varying torque
        double torqueNm = 5.0 + 2.0 * qSin(2.0 * M_PI * i / iterations);
        m_torqueDevice->setSimulatedTorque(torqueNm);

        // Simulate speed
        double speedRpm = 1000.0 + 200.0 * qCos(2.0 * M_PI * i / iterations);
        m_torqueDevice->setSimulatedSpeed(speedRpm);

        QThread::msleep(intervalMs);
    }
}

void MockDeviceManager::resetAllDevices() {
    if (m_motorDevice) {
        m_motorDevice->setSimulatedCurrent(0.0);
        m_motorDevice->setAI1InputLevel(false);
    }

    if (m_torqueDevice) {
        m_torqueDevice->setSimulatedTorque(0.0);
        m_torqueDevice->setSimulatedSpeed(0.0);
        m_torqueDevice->setSimulatedPower(0.0);
    }

    if (m_encoderDevice) {
        m_encoderDevice->setSimulatedAngle(0.0);
        m_encoderDevice->setSimulatedVelocity(0.0);
    }

    if (m_brakeDevice) {
        m_brakeDevice->setChannelMode(1, 0); // CC mode
        m_brakeDevice->setChannelMode(2, 0); // CC mode
    }
}

bool MockDeviceManager::isNearMagnet(double angleDeg, double magnetPos) const {
    double diff = qAbs(angleDeg - magnetPos);
    
    // Handle wrap-around at 360°
    if (diff > 180.0) {
        diff = 360.0 - diff;
    }
    
    return diff <= MAGNET_DETECTION_WINDOW;
}

} // namespace Simulation
} // namespace Infrastructure
