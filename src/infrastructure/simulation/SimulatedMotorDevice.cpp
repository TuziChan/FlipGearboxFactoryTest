#include "SimulatedMotorDevice.h"
#include <cmath>
#include <random>

namespace Infrastructure {
namespace Simulation {

SimulatedMotorDevice::SimulatedMotorDevice(SimulationContext* context, QObject* parent)
    : IMotorDriveDevice(parent)
    , m_context(context)
{
    // Initialize magnet detection state
    for (int i = 0; i < 3; i++) {
        m_magnetLastState[i] = false;
        m_magnetPassCounts[i] = 0;
    }
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

    m_context->incrementTickCount();

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

    m_context->incrementTickCount();

    // Get current and previous physical encoder angles (raw, not affected by zero offset)
    // Magnet positions are physical and should not change with encoder zero point
    double lastAngle = m_context->lastTickRawEncoderAngle();
    double currentAngle = m_context->rawEncoderAngleDeg();

    // Check if any magnet is within detection window OR was crossed during the last tick
    level = true; // Default: no magnet detected (HIGH)
    for (int i = 0; i < 3; i++) {
        bool inWindow = isAngleInWindow(currentAngle, MAGNET_POSITIONS[i], DETECTION_WINDOW);
        bool crossed = isMagnetCrossed(lastAngle, currentAngle, MAGNET_POSITIONS[i]);

        // Edge detection: count only on rising edge (entering window or crossing)
        if ((inWindow || crossed) && !m_magnetLastState[i]) {
            m_magnetPassCounts[i]++;
        }
        m_magnetLastState[i] = (inWindow || crossed);

        // If in any magnet window or crossed, set level to LOW
        if (inWindow || crossed) {
            level = false;
        }
    }

    return true;
}

bool SimulatedMotorDevice::isAngleInWindow(double angle, double targetAngle, double window) const {
    angle = normalizeAngle(angle);
    targetAngle = normalizeAngle(targetAngle);

    // Calculate shortest angular distance
    double diff = std::abs(angle - targetAngle);
    if (diff > 180.0) {
        diff = 360.0 - diff;
    }

    return diff <= window;
}

bool SimulatedMotorDevice::isMagnetCrossed(double fromAngle, double toAngle, double magnetPos) const {
    // Normalize all angles to 0-360 range
    fromAngle = normalizeAngle(fromAngle);
    toAngle = normalizeAngle(toAngle);
    magnetPos = normalizeAngle(magnetPos);

    // Calculate the angular distance traveled
    double delta = toAngle - fromAngle;

    // Handle wrap-around cases
    if (delta > 180.0) {
        delta -= 360.0;
    } else if (delta < -180.0) {
        delta += 360.0;
    }

    // Check if magnet position is between fromAngle and toAngle
    if (delta > 0.0) {
        // Moving forward
        if (fromAngle < toAngle) {
            // No wrap-around
            return (magnetPos > fromAngle && magnetPos < toAngle);
        } else {
            // Wrap-around case (e.g., 350° -> 10°)
            return (magnetPos > fromAngle || magnetPos < toAngle);
        }
    } else if (delta < 0.0) {
        // Moving backward
        if (fromAngle > toAngle) {
            // No wrap-around
            return (magnetPos < fromAngle && magnetPos > toAngle);
        } else {
            // Wrap-around case (e.g., 10° -> 350°)
            return (magnetPos < fromAngle || magnetPos > toAngle);
        }
    }

    return false; // No movement
}

double SimulatedMotorDevice::normalizeAngle(double angle) const {
    while (angle < 0.0) angle += 360.0;
    while (angle >= 360.0) angle -= 360.0;
    return angle;
}

QString SimulatedMotorDevice::lastError() const {
    return QString();
}

} // namespace Simulation
} // namespace Infrastructure
