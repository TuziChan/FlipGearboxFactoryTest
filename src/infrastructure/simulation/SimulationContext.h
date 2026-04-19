#ifndef SIMULATIONCONTEXT_H
#define SIMULATIONCONTEXT_H

#include <cstdint>
#include <cmath>

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

    uint64_t tickCount() const { return m_tickCount; }

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

    void setEncoderZeroOffset(double offset) { m_encoderZeroOffset = offset; }

    // Calculate angular velocity based on current simulated speed
    double encoderAngularVelocityRpm() const {
        return (m_motorDirection == MotorDirection::Forward) ? m_currentSpeedRpm : 
               (m_motorDirection == MotorDirection::Reverse) ? -m_currentSpeedRpm : 0.0;
    }

private:
    void updatePhysics() {
        // Simulate motor acceleration/deceleration with realistic dynamics
        if (m_motorDirection != MotorDirection::Stopped) {
            // Calculate brake load effect
            double brakeLoad = m_brakeOutputEnabled ? m_brakeCurrentA : 0.0;
            double speedReduction = brakeLoad * 100.0; // Each amp reduces speed by ~100 RPM
            double effectiveTargetSpeed = std::max(0.0, m_targetSpeedRpm - speedReduction);
            
            // Simulate acceleration/deceleration (realistic ramp)
            // Acceleration rate: ~500 RPM/s, which is ~5 RPM per tick (10ms)
            const double accelRatePerTick = 5.0;
            
            if (m_currentSpeedRpm < effectiveTargetSpeed) {
                m_currentSpeedRpm = std::min(m_currentSpeedRpm + accelRatePerTick, effectiveTargetSpeed);
            } else if (m_currentSpeedRpm > effectiveTargetSpeed) {
                // Deceleration is faster when brake is applied
                double decelRate = brakeLoad > 0.0 ? accelRatePerTick * 3.0 : accelRatePerTick;
                m_currentSpeedRpm = std::max(m_currentSpeedRpm - decelRate, effectiveTargetSpeed);
            }
            
            // Update encoder angle based on current speed
            // RPM to degrees per second: RPM * 360 / 60 = RPM * 6
            double degreesPerSecond = m_currentSpeedRpm * 6.0;
            double degreesPerTick = degreesPerSecond * 0.01; // 10ms per tick
            
            m_lastTickAngle = m_encoderAngleDeg;
            if (m_motorDirection == MotorDirection::Forward) {
                m_encoderAngleDeg += degreesPerTick;
            } else if (m_motorDirection == MotorDirection::Reverse) {
                m_encoderAngleDeg -= degreesPerTick;
            }
        } else {
            // Motor stopped - apply friction deceleration
            if (m_currentSpeedRpm > 0.0) {
                const double frictionDecelPerTick = 2.0; // Natural deceleration
                m_currentSpeedRpm = std::max(0.0, m_currentSpeedRpm - frictionDecelPerTick);
            }
            m_targetSpeedRpm = 0.0;
        }
    }

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
