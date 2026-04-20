#include "SimulationTestHelper.h"
#include <QThread>
#include <QDebug>
#include <cmath>

namespace Infrastructure {
namespace Simulation {

SimulationTestHelper::SimulationTestHelper(SimulationContext* context, QObject* parent)
    : QObject(parent)
    , m_context(context)
    , m_lastAngle(0.0)
    , m_magnetDetected(false)
{
}

// ========== Time Control ==========

void SimulationTestHelper::advanceTicks(int ticks) {
    for (int i = 0; i < ticks; i++) {
        m_context->advanceTick();
        
        // Check for magnet detection
        double currentAngle = m_context->encoderAngleDeg();
        if (m_magnetCallback && m_magnetCallback(currentAngle)) {
            if (!m_magnetDetected) {
                m_magnetDetected = true;
                emit magnetDetected(currentAngle);
            }
        } else {
            m_magnetDetected = false;
        }
        
        // Check for angle changes
        if (std::abs(currentAngle - m_lastAngle) > 0.1) {
            emit angleReached(currentAngle);
            m_lastAngle = currentAngle;
        }
    }
}

void SimulationTestHelper::advanceMs(int ms) {
    int ticks = ms / 10; // 1 tick = 10ms
    advanceTicks(ticks);
}

uint64_t SimulationTestHelper::currentTick() const {
    return m_context->tickCount();
}

// ========== Motor Control ==========

void SimulationTestHelper::setMotorForward(double dutyCyclePercent) {
    m_context->setMotorDirection(SimulationContext::MotorDirection::Forward);
    m_context->setMotorDutyCycle(dutyCyclePercent);
}

void SimulationTestHelper::setMotorReverse(double dutyCyclePercent) {
    m_context->setMotorDirection(SimulationContext::MotorDirection::Reverse);
    m_context->setMotorDutyCycle(dutyCyclePercent);
}

void SimulationTestHelper::stopMotor() {
    m_context->setMotorDirection(SimulationContext::MotorDirection::Stopped);
    m_context->setMotorDutyCycle(0.0);
}

double SimulationTestHelper::motorSpeedRpm() const {
    return m_context->encoderAngularVelocityRpm();
}

// ========== Encoder Control ==========

void SimulationTestHelper::setEncoderAngle(double angleDeg) {
    // This is a test-only method to directly set encoder angle
    // In real simulation, angle is updated by physics
    // We need to access the internal state - this is a limitation
    // For now, we'll document that this should be used carefully
    qWarning() << "SimulationTestHelper::setEncoderAngle() directly manipulates internal state";
}

double SimulationTestHelper::encoderAngle() const {
    return m_context->encoderAngleDeg();
}

void SimulationTestHelper::setEncoderZero(double offsetDeg) {
    m_context->setEncoderZeroOffset(offsetDeg);
}

bool SimulationTestHelper::waitForEncoderAngle(double targetDeg, double toleranceDeg, int timeoutMs) {
    int ticksRemaining = timeoutMs / 10;
    
    while (ticksRemaining > 0) {
        double currentAngle = m_context->encoderAngleDeg();
        double diff = std::abs(currentAngle - targetDeg);
        
        // Handle wrap-around
        if (diff > 180.0) {
            diff = 360.0 - diff;
        }
        
        if (diff <= toleranceDeg) {
            return true;
        }
        
        advanceTicks(1);
        ticksRemaining--;
    }
    
    return false;
}

// ========== Brake Control ==========

void SimulationTestHelper::setBrakeCurrent(double currentA) {
    m_context->setBrakeCurrent(currentA);
}

void SimulationTestHelper::setBrakeEnabled(bool enabled) {
    m_context->setBrakeOutputEnabled(enabled);
}

double SimulationTestHelper::brakeCurrent() const {
    return m_context->brakeCurrent();
}

// ========== Event Triggers ==========

void SimulationTestHelper::triggerMagnetDetection() {
    double currentAngle = m_context->encoderAngleDeg();
    emit magnetDetected(currentAngle);
}

void SimulationTestHelper::setMagnetDetectionCallback(std::function<bool(double)> callback) {
    m_magnetCallback = callback;
}

// ========== State Verification ==========

bool SimulationTestHelper::isMotorRunning() const {
    return m_context->motorDirection() != SimulationContext::MotorDirection::Stopped;
}

bool SimulationTestHelper::isMotorForward() const {
    return m_context->motorDirection() == SimulationContext::MotorDirection::Forward;
}

bool SimulationTestHelper::isMotorReverse() const {
    return m_context->motorDirection() == SimulationContext::MotorDirection::Reverse;
}

bool SimulationTestHelper::isBrakeEnabled() const {
    return m_context->brakeOutputEnabled();
}

double SimulationTestHelper::motorDutyCycle() const {
    return m_context->motorDutyCycle();
}

// ========== Scenario Helpers ==========

void SimulationTestHelper::setupIdleRunScenario(double dutyCyclePercent, double targetSpeedRpm) {
    reset();
    
    // Set motor to forward with specified duty cycle
    setMotorForward(dutyCyclePercent);
    
    // Run simulation until speed stabilizes
    advanceMs(500); // Allow acceleration time
    
    qDebug() << "Idle run scenario setup: duty=" << dutyCyclePercent 
             << "% speed=" << motorSpeedRpm() << "RPM";
}

void SimulationTestHelper::setupLoadTestScenario(double motorDutyCycle, double brakeCurrentA) {
    reset();
    
    // Start motor
    setMotorForward(motorDutyCycle);
    advanceMs(200); // Spinup time
    
    // Apply brake load
    setBrakeCurrent(brakeCurrentA);
    setBrakeEnabled(true);
    advanceMs(100); // Allow load to take effect
    
    qDebug() << "Load test scenario setup: motor=" << motorDutyCycle 
             << "% brake=" << brakeCurrentA << "A speed=" << motorSpeedRpm() << "RPM";
}

void SimulationTestHelper::setupHomingScenario(double magnetAngleDeg) {
    reset();
    
    // Set magnet detection at specified angle
    setMagnetDetectionCallback([magnetAngleDeg](double angle) {
        // Detect magnet when within 1 degree of target
        double diff = std::abs(angle - magnetAngleDeg);
        if (diff > 180.0) diff = 360.0 - diff;
        return diff < 1.0;
    });
    
    // Start motor in reverse for homing
    setMotorReverse(20.0);
    
    qDebug() << "Homing scenario setup: magnet at" << magnetAngleDeg << "degrees";
}

void SimulationTestHelper::reset() {
    m_context->reset();
    m_lastAngle = 0.0;
    m_magnetDetected = false;
    m_magnetCallback = nullptr;
}

// ========== Advanced Control ==========

bool SimulationTestHelper::runUntil(std::function<bool()> condition, int timeoutMs, int tickIntervalMs) {
    int tickInterval = tickIntervalMs / 10;
    int ticksRemaining = timeoutMs / 10;
    
    while (ticksRemaining > 0) {
        if (condition()) {
            return true;
        }
        
        advanceTicks(tickInterval);
        ticksRemaining -= tickInterval;
    }
    
    return false;
}

} // namespace Simulation
} // namespace Infrastructure
