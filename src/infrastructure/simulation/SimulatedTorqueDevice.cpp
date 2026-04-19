#include "SimulatedTorqueDevice.h"
#include <cmath>
#include <random>

namespace Infrastructure {
namespace Simulation {

SimulatedTorqueDevice::SimulatedTorqueDevice(SimulationContext* context, QObject* parent)
    : ITorqueSensorDevice(parent)
    , m_context(context)
{
}

bool SimulatedTorqueDevice::initialize() {
    return true;
}

bool SimulatedTorqueDevice::readTorque(double& torqueNm) {
    if (!m_context) return false;

    m_context->advanceTick();

    // Calculate realistic torque based on motor state and brake load
    double torque = calculateTorque();
    
    // Add small noise to simulate real sensor
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::normal_distribution<> noise(0.0, 0.05); // ±0.05 N·m noise
    
    torqueNm = torque + noise(gen);
    return true;
}

bool SimulatedTorqueDevice::readSpeed(double& speedRpm) {
    if (!m_context) return false;

    m_context->advanceTick();

    // Get speed from encoder calculation
    speedRpm = std::abs(m_context->encoderAngularVelocityRpm());
    return true;
}

bool SimulatedTorqueDevice::readPower(double& powerW) {
    if (!m_context) return false;

    m_context->advanceTick();

    double torque = calculateTorque();
    double speed = std::abs(m_context->encoderAngularVelocityRpm());

    // Power = Torque * AngularVelocity
    // P(W) = T(N·m) * ω(rad/s) = T(N·m) * (RPM * 2π/60)
    powerW = torque * speed * 2.0 * M_PI / 60.0;

    return true;
}

bool SimulatedTorqueDevice::readAll(double& torqueNm, double& speedRpm, double& powerW) {
    if (!m_context) return false;

    m_context->advanceTick();

    // Calculate torque
    torqueNm = calculateTorque();
    
    // Add small noise
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::normal_distribution<> noise(0.0, 0.05);
    torqueNm += noise(gen);

    // Get speed
    speedRpm = std::abs(m_context->encoderAngularVelocityRpm());

    // Calculate power
    powerW = torqueNm * speedRpm * 2.0 * M_PI / 60.0;

    return true;
}

double SimulatedTorqueDevice::calculateTorque() const {
    // Base friction torque (always present when motor is running)
    double baseTorque = 0.3;
    
    // Motor load torque (proportional to duty cycle and speed)
    double motorTorque = 0.0;
    if (m_context->motorDirection() != SimulationContext::MotorDirection::Stopped) {
        double dutyCycle = m_context->motorDutyCycle();
        double speed = std::abs(m_context->encoderAngularVelocityRpm());
        
        // Torque increases with duty cycle but decreases slightly at higher speeds
        // Typical motor characteristic: T = k1 * duty - k2 * speed
        motorTorque = (dutyCycle / 100.0) * 2.0 - (speed / 1000.0) * 0.5;
        motorTorque = std::max(0.0, motorTorque);
    }
    
    // Brake load torque (dominant during load test)
    double brakeTorque = 0.0;
    if (m_context->brakeOutputEnabled()) {
        // Torque is proportional to brake current
        // Typical: ~0.8-1.0 N·m per amp of brake current
        brakeTorque = m_context->brakeCurrent() * 0.85;
    }
    
    // Total torque is sum of all components
    double totalTorque = baseTorque + motorTorque + brakeTorque;
    
    // Clamp to realistic range (0-50 N·m for this gearbox)
    return std::clamp(totalTorque, 0.0, 50.0);
}

QString SimulatedTorqueDevice::lastError() const {
    return QString();
}

} // namespace Simulation
} // namespace Infrastructure
