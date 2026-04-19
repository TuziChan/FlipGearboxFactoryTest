#include "SimulatedMotorDevice.h"
#include <cmath>
#include <random>

namespace Infrastructure {
namespace Simulation {

SimulatedMotorDevice::SimulatedMotorDevice(SimulationContext* context, QObject* parent)
    : IMotorDriveDevice(parent)
    , m_context(context)
    , m_ai1TransitionTick(AI1_TRANSITION_DELAY)
{
}

bool SimulatedMotorDevice::initialize() {
    return true;
}

bool SimulatedMotorDevice::setMotor(Direction direction, double dutyCyclePercent) {
    if (!m_context) return false;

    // Update simulation context
    switch (direction) {
        case Direction::Forward:
            m_context->setMotorDirection(SimulationContext::MotorDirection::Forward);
            break;
        case Direction::Reverse:
            m_context->setMotorDirection(SimulationContext::MotorDirection::Reverse);
            break;
        case Direction::Brake:
            m_context->setMotorDirection(SimulationContext::MotorDirection::Stopped);
            break;
    }

    m_context->setMotorDutyCycle(dutyCyclePercent);
    return true;
}

bool SimulatedMotorDevice::brake() {
    if (!m_context) return false;
    m_context->setMotorDirection(SimulationContext::MotorDirection::Stopped);
    m_context->setMotorDutyCycle(0.0);
    return true;
}

bool SimulatedMotorDevice::coast() {
    if (!m_context) return false;
    m_context->setMotorDirection(SimulationContext::MotorDirection::Stopped);
    m_context->setMotorDutyCycle(0.0);
    return true;
}

bool SimulatedMotorDevice::readCurrent(double& currentA) {
    if (!m_context) return false;

    m_context->advanceTick();

    // Calculate realistic motor current based on duty cycle, speed, and load
    double dutyCycle = m_context->motorDutyCycle();
    double speed = std::abs(m_context->encoderAngularVelocityRpm());
    double brakeLoad = m_context->brakeOutputEnabled() ? m_context->brakeCurrent() : 0.0;
    
    // Base idle current
    double baseCurrent = 0.5;
    
    // Current increases with duty cycle
    double dutyCycleCurrent = (dutyCycle / 100.0) * 1.5;
    
    // Current increases with brake load (motor works harder)
    double loadCurrent = brakeLoad * 0.3;
    
    // Current decreases slightly at higher speeds (back-EMF effect)
    double speedReduction = (speed / 1000.0) * 0.2;
    
    currentA = baseCurrent + dutyCycleCurrent + loadCurrent - speedReduction;
    currentA = std::max(0.3, currentA); // Minimum current
    
    // Add small noise to simulate real sensor
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::normal_distribution<> noise(0.0, 0.02); // ±0.02A noise
    currentA += noise(gen);
    
    return true;
}

bool SimulatedMotorDevice::readAI1Level(bool& level) {
    if (!m_context) return false;

    m_context->advanceTick();

    // AI1 starts high (no magnet), transitions to low (magnet detected) after N ticks
    // This simulates the motor moving until the magnet is detected
    if (m_context->tickCount() < m_ai1TransitionTick) {
        level = true; // No magnet detected yet
    } else {
        level = false; // Magnet detected
    }

    return true;
}

QString SimulatedMotorDevice::lastError() const {
    return QString();
}

} // namespace Simulation
} // namespace Infrastructure
