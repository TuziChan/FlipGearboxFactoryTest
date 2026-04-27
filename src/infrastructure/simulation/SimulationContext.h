#ifndef SIMULATIONCONTEXT_H
#define SIMULATIONCONTEXT_H

#include <cstdint>
#include <cmath>
#include <algorithm>

namespace Infrastructure {
namespace Simulation {

/**
 * @brief Shared simulation state for all simulated devices
 * 
 * This class holds the physical state of the simulated system.
 * All simulated devices share a pointer to this context and update/read from it.
 */
class SimulationContext {
public:
    enum class MotorDirection {
        Stopped,
        Forward,
        Reverse
    };

    SimulationContext()
        : m_tickCount(0)
        , m_motorDirection(MotorDirection::Stopped)
        , m_motorDutyCycle(0.0)
        , m_brakeCurrentA(0.0)
        , m_brakeVoltageV(0.0)
        , m_brakeOutputEnabled(false)
        , m_encoderAngleDeg(0.0)
        , m_encoderZeroOffset(0.0)
        , m_lastTickAngle(0.0)
        , m_currentSpeedRpm(0.0)
        , m_targetSpeedRpm(0.0)
    {
    }

    // Tick management
    void advanceTick() {
        m_tickCount++;
        updatePhysics();
    }

    /// Increment tick counter only (no physics update).
    /// Used by device read methods so they don't advance the simulation.
    void incrementTickCount() {
        m_tickCount++;
    }

    uint64_t tickCount() const { return m_tickCount; }

    /// Get the encoder angle from before the last physics update
    double lastTickRawEncoderAngle() const { return m_lastTickAngle; }

    void reset() {
        m_tickCount = 0;
        m_motorDirection = MotorDirection::Stopped;
        m_motorDutyCycle = 0.0;
        m_brakeCurrentA = 0.0;
        m_brakeVoltageV = 0.0;
        m_brakeOutputEnabled = false;
        m_encoderAngleDeg = 0.0;
        m_encoderZeroOffset = 0.0;
        m_lastTickAngle = 0.0;
        m_currentSpeedRpm = 0.0;
        m_targetSpeedRpm = 0.0;
    }

    // Motor state
    void setMotorDirection(MotorDirection direction) { m_motorDirection = direction; }
    MotorDirection motorDirection() const { return m_motorDirection; }

    void setMotorDutyCycle(double dutyCycle) { 
        m_motorDutyCycle = dutyCycle;
        // Update target speed based on duty cycle
        if (m_motorDirection != MotorDirection::Stopped) {
            m_targetSpeedRpm = dutyCycle * 15.0; // Max ~1500 RPM at 100% duty
        } else {
            m_targetSpeedRpm = 0.0;
        }
    }
    double motorDutyCycle() const { return m_motorDutyCycle; }

    // Brake state
    void setBrakeCurrent(double currentA) { m_brakeCurrentA = currentA; }
    double brakeCurrent() const { return m_brakeCurrentA; }

    void setBrakeVoltage(double voltageV) { m_brakeVoltageV = voltageV; }
    double brakeVoltage() const { return m_brakeVoltageV; }

    void setBrakeOutputEnabled(bool enabled) { m_brakeOutputEnabled = enabled; }
    bool brakeOutputEnabled() const { return m_brakeOutputEnabled; }

    // Encoder state
    double encoderAngleDeg() const {
        // Return angle relative to zero point, wrapped to 0-360
        double angle = m_encoderAngleDeg - m_encoderZeroOffset;
        while (angle < 0.0) angle += 360.0;
        while (angle >= 360.0) angle -= 360.0;
        return angle;
    }

    double encoderTotalAngleDeg() const {
        return m_encoderAngleDeg - m_encoderZeroOffset;
    }

    double rawEncoderAngleDeg() const {
        return m_encoderAngleDeg;
    }

    void setEncoderZeroOffset(double offset) { m_encoderZeroOffset = offset; }

    // Calculate angular velocity based on current simulated speed
    double encoderAngularVelocityRpm() const {
        return (m_motorDirection == MotorDirection::Forward) ? m_currentSpeedRpm : 
               (m_motorDirection == MotorDirection::Reverse) ? -m_currentSpeedRpm : 0.0;
    }

private:
    void updatePhysics() {
        // Save angle before physics update for crossing detection
        m_lastTickAngle = m_encoderAngleDeg;

        // Use sub-stepping to avoid skipping over magnet detection windows
        // 10 sub-steps of 1ms each = 10ms total per tick
        const int subSteps = 10;
        const double subStepTimeS = 0.001; // 1ms per sub-step

        for (int i = 0; i < subSteps; i++) {
            // Simulate motor acceleration/deceleration with realistic dynamics
            if (m_motorDirection != MotorDirection::Stopped) {
                // Calculate brake load effect on deceleration rate
                double brakeLoad = m_brakeOutputEnabled ? m_brakeCurrentA : 0.0;

                // Simulate acceleration/deceleration (realistic ramp)
                // Acceleration rate: ~500 RPM/s, which is ~0.5 RPM per 1ms sub-step
                const double accelRatePerSubStep = 0.5;

                if (m_currentSpeedRpm < m_targetSpeedRpm) {
                    m_currentSpeedRpm = std::min(m_currentSpeedRpm + accelRatePerSubStep, m_targetSpeedRpm);
                } else if (m_currentSpeedRpm > m_targetSpeedRpm) {
                    // Deceleration is faster when brake is applied (3x rate)
                    double decelRate = brakeLoad > 0.0 ? accelRatePerSubStep * 3.0 : accelRatePerSubStep;
                    m_currentSpeedRpm = std::max(m_currentSpeedRpm - decelRate, m_targetSpeedRpm);
                }

                // Update encoder angle based on current speed
                // RPM to degrees per second: RPM * 360 / 60 = RPM * 6
                double degreesPerSecond = m_currentSpeedRpm * 6.0;
                double degreesPerSubStep = degreesPerSecond * subStepTimeS;

                if (m_motorDirection == MotorDirection::Forward) {
                    m_encoderAngleDeg += degreesPerSubStep;
                } else if (m_motorDirection == MotorDirection::Reverse) {
                    m_encoderAngleDeg -= degreesPerSubStep;
                }
            } else {
                // Motor stopped - apply friction deceleration
                if (m_currentSpeedRpm > 0.0) {
                    const double frictionDecelPerSubStep = 0.2; // Natural deceleration per sub-step
                    m_currentSpeedRpm = std::max(0.0, m_currentSpeedRpm - frictionDecelPerSubStep);
                }
                m_targetSpeedRpm = 0.0;
            }
        }
    }

protected:
    uint64_t m_tickCount;
    MotorDirection m_motorDirection;
    double m_motorDutyCycle;
    double m_brakeCurrentA;
    double m_brakeVoltageV;
    bool m_brakeOutputEnabled;
    double m_encoderAngleDeg;
    double m_encoderZeroOffset;
    double m_lastTickAngle;
    double m_currentSpeedRpm;  // Current actual speed (with acceleration/deceleration)
    double m_targetSpeedRpm;   // Target speed based on duty cycle
};

} // namespace Simulation
} // namespace Infrastructure

#endif // SIMULATIONCONTEXT_H
