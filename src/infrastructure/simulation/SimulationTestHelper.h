#ifndef SIMULATIONTESTHELPER_H
#define SIMULATIONTESTHELPER_H

#include "SimulationContext.h"
#include <QObject>
#include <functional>

namespace Infrastructure {
namespace Simulation {

/**
 * @brief Test helper for simulation runtime
 * 
 * Provides convenient APIs for controlling simulation state in tests:
 * - Deterministic time advancement
 * - Event triggering (magnet detection, angle arrival)
 * - State inspection and verification
 * - Scenario setup helpers
 */
class SimulationTestHelper : public QObject {
    Q_OBJECT

public:
    explicit SimulationTestHelper(SimulationContext* context, QObject* parent = nullptr);

    // ========== Time Control ==========
    
    /**
     * @brief Advance simulation by specified number of ticks
     * @param ticks Number of ticks to advance (1 tick = 10ms)
     */
    void advanceTicks(int ticks);
    
    /**
     * @brief Advance simulation by specified milliseconds
     * @param ms Milliseconds to advance
     */
    void advanceMs(int ms);
    
    /**
     * @brief Get current tick count
     */
    uint64_t currentTick() const;

    // ========== Motor Control ==========
    
    /**
     * @brief Set motor to forward direction with duty cycle
     */
    void setMotorForward(double dutyCyclePercent);
    
    /**
     * @brief Set motor to reverse direction with duty cycle
     */
    void setMotorReverse(double dutyCyclePercent);
    
    /**
     * @brief Stop motor
     */
    void stopMotor();
    
    /**
     * @brief Get current motor speed in RPM
     */
    double motorSpeedRpm() const;

    // ========== Encoder Control ==========
    
    /**
     * @brief Set encoder angle directly (for testing)
     * @param angleDeg Angle in degrees (0-360)
     */
    void setEncoderAngle(double angleDeg);
    
    /**
     * @brief Get current encoder angle
     */
    double encoderAngle() const;
    
    /**
     * @brief Set encoder zero offset
     */
    void setEncoderZero(double offsetDeg);
    
    /**
     * @brief Wait until encoder reaches target angle (with timeout)
     * @param targetDeg Target angle in degrees
     * @param toleranceDeg Tolerance in degrees
     * @param timeoutMs Timeout in milliseconds
     * @return true if angle reached within timeout
     */
    bool waitForEncoderAngle(double targetDeg, double toleranceDeg, int timeoutMs);

    // ========== Brake Control ==========
    
    /**
     * @brief Set brake current
     */
    void setBrakeCurrent(double currentA);
    
    /**
     * @brief Enable/disable brake output
     */
    void setBrakeEnabled(bool enabled);
    
    /**
     * @brief Get current brake current
     */
    double brakeCurrent() const;

    // ========== Event Triggers ==========
    
    /**
     * @brief Trigger magnet detection event
     * 
     * Simulates the motor passing over a magnet sensor.
     * This is useful for testing homing sequences.
     */
    void triggerMagnetDetection();
    
    /**
     * @brief Set magnet detection callback
     * 
     * The callback will be invoked when magnet should be detected
     * based on encoder position.
     * 
     * @param callback Function that returns true when magnet should be detected
     */
    void setMagnetDetectionCallback(std::function<bool(double angleDeg)> callback);

    // ========== State Verification ==========
    
    /**
     * @brief Check if motor is running
     */
    bool isMotorRunning() const;
    
    /**
     * @brief Check if motor is in forward direction
     */
    bool isMotorForward() const;
    
    /**
     * @brief Check if motor is in reverse direction
     */
    bool isMotorReverse() const;
    
    /**
     * @brief Check if brake is enabled
     */
    bool isBrakeEnabled() const;
    
    /**
     * @brief Get motor duty cycle
     */
    double motorDutyCycle() const;

    // ========== Scenario Helpers ==========
    
    /**
     * @brief Setup idle run scenario
     * 
     * Configures simulation for typical idle run test:
     * - Motor at specified duty cycle
     * - Stable speed
     * - Low brake load
     */
    void setupIdleRunScenario(double dutyCyclePercent, double targetSpeedRpm);
    
    /**
     * @brief Setup load test scenario
     * 
     * Configures simulation for load test:
     * - Motor running
     * - Brake applying load
     * - Speed reduction due to load
     */
    void setupLoadTestScenario(double motorDutyCycle, double brakeCurrentA);
    
    /**
     * @brief Setup homing scenario
     * 
     * Configures simulation for homing test:
     * - Motor in reverse
     * - Magnet detection at specified angle
     */
    void setupHomingScenario(double magnetAngleDeg);
    
    /**
     * @brief Reset simulation to initial state
     */
    void reset();

    // ========== Advanced Control ==========
    
    /**
     * @brief Run simulation until condition is met or timeout
     * 
     * @param condition Function that returns true when condition is met
     * @param timeoutMs Timeout in milliseconds
     * @param tickIntervalMs Tick interval for checking condition
     * @return true if condition met, false if timeout
     */
    bool runUntil(std::function<bool()> condition, int timeoutMs, int tickIntervalMs = 10);
    
    /**
     * @brief Get simulation context (for advanced usage)
     */
    SimulationContext* context() const { return m_context; }

signals:
    /**
     * @brief Emitted when magnet is detected
     */
    void magnetDetected(double angleDeg);
    
    /**
     * @brief Emitted when encoder reaches target angle
     */
    void angleReached(double angleDeg);

private:
    SimulationContext* m_context;
    std::function<bool(double)> m_magnetCallback;
    double m_lastAngle;
    bool m_magnetDetected;
};

} // namespace Simulation
} // namespace Infrastructure

#endif // SIMULATIONTESTHELPER_H
