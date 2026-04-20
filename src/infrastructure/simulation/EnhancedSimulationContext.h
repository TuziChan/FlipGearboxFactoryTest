#ifndef ENHANCEDSIMULATIONCONTEXT_H
#define ENHANCEDSIMULATIONCONTEXT_H

#include "SimulationContext.h"
#include <QRandomGenerator>
#include <QVector>

namespace Infrastructure {
namespace Simulation {

/**
 * @brief Enhanced simulation context with fault injection and boundary testing
 * 
 * Extends SimulationContext to support:
 * - Boundary conditions (min/max values)
 * - Sensor disconnection simulation
 * - Data timeout simulation
 * - Data jump/glitch injection
 * - Magnet position configuration for angle tests
 */
class EnhancedSimulationContext : public SimulationContext {
public:
    enum class FaultType {
        None,
        SensorDisconnected,    // Sensor returns invalid/max values
        DataTimeout,           // Sensor stops updating
        DataJump,              // Sudden large value changes
        NoiseInjection,        // Random noise on readings
        StuckValue,            // Value doesn't change
        OutOfRange             // Values exceed physical limits
    };

    EnhancedSimulationContext()
        : SimulationContext()
        , m_encoderFault(FaultType::None)
        , m_torqueFault(FaultType::None)
        , m_speedFault(FaultType::None)
        , m_stuckEncoderValue(0.0)
        , m_stuckTorqueValue(0.0)
        , m_stuckSpeedValue(0.0)
        , m_dataTimeoutTicks(0)
        , m_simulatedTorqueNm(0.0)
        , m_forceSpeedOverride(false)
        , m_overrideSpeedRpm(0.0)
    {
        // Default magnet positions for angle test (3°, 49°, 113°)
        m_magnetPositions = {3.0, 49.0, 113.0};
    }

    // ========== Fault Injection ==========
    
    void setEncoderFault(FaultType fault) { 
        m_encoderFault = fault;
        if (fault == FaultType::StuckValue) {
            m_stuckEncoderValue = encoderAngleDeg();
        }
    }
    
    void setTorqueFault(FaultType fault) { 
        m_torqueFault = fault;
        if (fault == FaultType::StuckValue) {
            m_stuckTorqueValue = m_simulatedTorqueNm;
        }
    }
    
    void setSpeedFault(FaultType fault) { 
        m_speedFault = fault;
        if (fault == FaultType::StuckValue) {
            m_stuckSpeedValue = encoderAngularVelocityRpm();
        }
    }

    FaultType encoderFault() const { return m_encoderFault; }
    FaultType torqueFault() const { return m_torqueFault; }
    FaultType speedFault() const { return m_speedFault; }

    // ========== Boundary Value Control ==========
    
    void forceEncoderAngle(double angleDeg) {
        // Directly set encoder angle (for boundary testing)
        m_encoderAngleDeg = angleDeg;
    }
    
    void forceSpeed(double speedRpm) {
        m_forceSpeedOverride = true;
        m_overrideSpeedRpm = speedRpm;
    }
    
    void clearSpeedOverride() {
        m_forceSpeedOverride = false;
    }

    // ========== Magnet Position Configuration ==========
    
    void setMagnetPositions(const QVector<double>& positions) {
        m_magnetPositions = positions;
    }
    
    QVector<double> magnetPositions() const {
        return m_magnetPositions;
    }
    
    bool isMagnetDetectedAt(double angleDeg, double toleranceDeg = 1.0) const {
        for (double magnetPos : m_magnetPositions) {
            double diff = std::abs(angleDeg - magnetPos);
            // Handle wrap-around
            if (diff > 180.0) {
                diff = 360.0 - diff;
            }
            if (diff <= toleranceDeg) {
                return true;
            }
        }
        return false;
    }

    // ========== Enhanced Readings with Fault Injection ==========
    
    double getEncoderAngleWithFaults() const {
        switch (m_encoderFault) {
            case FaultType::None:
                return encoderAngleDeg();
                
            case FaultType::SensorDisconnected:
                return 999.9; // Invalid value indicating disconnection
                
            case FaultType::DataTimeout:
                // Return stale value
                return m_stuckEncoderValue;
                
            case FaultType::DataJump:
                // Random jump ±50 degrees
                return encoderAngleDeg() + (QRandomGenerator::global()->bounded(100) - 50);
                
            case FaultType::NoiseInjection:
                // Add ±2 degree noise
                return encoderAngleDeg() + (QRandomGenerator::global()->bounded(40) - 20) / 10.0;
                
            case FaultType::StuckValue:
                return m_stuckEncoderValue;
                
            case FaultType::OutOfRange:
                return 400.0; // Beyond 0-360 range
        }
        return encoderAngleDeg();
    }
    
    double getSpeedWithFaults() const {
        double baseSpeed = m_forceSpeedOverride ? m_overrideSpeedRpm : encoderAngularVelocityRpm();
        
        switch (m_speedFault) {
            case FaultType::None:
                return baseSpeed;
                
            case FaultType::SensorDisconnected:
                return -999.9; // Invalid value
                
            case FaultType::DataTimeout:
                return m_stuckSpeedValue;
                
            case FaultType::DataJump:
                // Random jump ±500 RPM
                return baseSpeed + (QRandomGenerator::global()->bounded(1000) - 500);
                
            case FaultType::NoiseInjection:
                // Add ±10 RPM noise
                return baseSpeed + (QRandomGenerator::global()->bounded(20) - 10);
                
            case FaultType::StuckValue:
                return m_stuckSpeedValue;
                
            case FaultType::OutOfRange:
                return 10000.0; // Unrealistic speed
        }
        return baseSpeed;
    }
    
    double getTorqueWithFaults() const {
        // Calculate simulated torque based on brake load
        double baseTorque = m_simulatedTorqueNm;
        
        switch (m_torqueFault) {
            case FaultType::None:
                return baseTorque;
                
            case FaultType::SensorDisconnected:
                return -999.9; // Invalid value
                
            case FaultType::DataTimeout:
                return m_stuckTorqueValue;
                
            case FaultType::DataJump:
                // Random jump ±5 Nm
                return baseTorque + (QRandomGenerator::global()->bounded(100) - 50) / 10.0;
                
            case FaultType::NoiseInjection:
                // Add ±0.5 Nm noise
                return baseTorque + (QRandomGenerator::global()->bounded(10) - 5) / 10.0;
                
            case FaultType::StuckValue:
                return m_stuckTorqueValue;
                
            case FaultType::OutOfRange:
                return 1000.0; // Unrealistic torque
        }
        return baseTorque;
    }

    // ========== Torque Simulation ==========
    
    void updateTorqueSimulation() {
        // Simulate torque based on brake current and motor speed
        if (brakeOutputEnabled() && brakeCurrent() > 0.0) {
            // Torque increases with brake current, decreases with speed
            double speedFactor = std::max(0.1, 1.0 - encoderAngularVelocityRpm() / 2000.0);
            m_simulatedTorqueNm = brakeCurrent() * 2.0 * speedFactor; // ~2 Nm per amp
        } else {
            m_simulatedTorqueNm = 0.0;
        }
    }

    // ========== Override advanceTick to update torque ==========
    
    void advanceTick() {
        SimulationContext::advanceTick();
        updateTorqueSimulation();
        
        // Update timeout counter
        if (m_encoderFault == FaultType::DataTimeout ||
            m_torqueFault == FaultType::DataTimeout ||
            m_speedFault == FaultType::DataTimeout) {
            m_dataTimeoutTicks++;
        }
    }

    // ========== Statistics ==========
    
    void resetFaults() {
        m_encoderFault = FaultType::None;
        m_torqueFault = FaultType::None;
        m_speedFault = FaultType::None;
        m_dataTimeoutTicks = 0;
        m_forceSpeedOverride = false;
    }

private:
    FaultType m_encoderFault;
    FaultType m_torqueFault;
    FaultType m_speedFault;
    
    double m_stuckEncoderValue;
    double m_stuckTorqueValue;
    double m_stuckSpeedValue;
    
    uint64_t m_dataTimeoutTicks;
    
    QVector<double> m_magnetPositions;
    double m_simulatedTorqueNm;
    
    bool m_forceSpeedOverride;
    double m_overrideSpeedRpm;
};

} // namespace Simulation
} // namespace Infrastructure

#endif // ENHANCEDSIMULATIONCONTEXT_H
