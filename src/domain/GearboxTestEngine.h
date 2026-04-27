#ifndef GEARBOXTESTENGINE_H
#define GEARBOXTESTENGINE_H

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include "TestRecipe.h"
#include "TestRunState.h"
#include "TestResults.h"
#include "TelemetrySnapshot.h"
#include "../infrastructure/devices/IMotorDriveDevice.h"
#include "../infrastructure/devices/ITorqueSensorDevice.h"
#include "../infrastructure/devices/IEncoderDevice.h"
#include "../infrastructure/devices/IBrakePowerDevice.h"
#include "../infrastructure/acquisition/AcquisitionScheduler.h"
#include "../infrastructure/validation/PhysicsValidator.h"
#include "../infrastructure/validation/PhysicsViolationLogger.h"

// Forward declaration for optional simulation context
namespace Infrastructure {
namespace Simulation {
class SimulationContext;
}
}

namespace Domain {

/**
 * @brief Core test engine state machine
 * 
 * Orchestrates the complete gearbox factory test flow:
 * 1. Homing with magnet detection
 * 2. Idle run (forward/reverse)
 * 3. Angle positioning (5 measurements)
 * 4. Load test with brake ramp (forward/reverse)
 * 5. Return to zero
 */
class GearboxTestEngine : public QObject {
    Q_OBJECT

public:
    explicit GearboxTestEngine(QObject* parent = nullptr);
    ~GearboxTestEngine() override = default;

    /**
     * @brief Set device references (must be called before start)
     */
    void setDevices(Infrastructure::Devices::IMotorDriveDevice* motor,
                    Infrastructure::Devices::ITorqueSensorDevice* torque,
                    Infrastructure::Devices::IEncoderDevice* encoder,
                    Infrastructure::Devices::IBrakePowerDevice* brake);

    void setAcquisitionScheduler(Infrastructure::Acquisition::AcquisitionScheduler* scheduler);

    /**
     * @brief Set test recipe
     */
    void setRecipe(const TestRecipe& recipe);
    void setBrakeChannel(int channel);
    void setStationName(const QString& stationName);

    /**
     * @brief Enable/disable physics validation (default: enabled)
     */
    void setPhysicsValidationEnabled(bool enabled);

    /**
     * @brief Configure physics validation thresholds
     */
    void setValidationConfig(const Infrastructure::Validation::PhysicsValidator::ValidationConfig& config);

    /**
     * @brief Set simulation context for mock mode tick advancement
     * 
     * When set, the engine will advance the simulation physics once per
     * cycle tick, ensuring consistent angle updates regardless of how
     * many device reads occur within a single tick.
     */
    void setSimulationContext(Infrastructure::Simulation::SimulationContext* context);

    /**
     * @brief Start test with given serial number
     */
    bool startTest(const QString& serialNumber);

    /**
     * @brief Emergency stop
     */
    void emergencyStop();

    /**
     * @brief Reset to idle state
     */
    void reset();

    /**
     * @brief Get current state
     */
    TestRunState currentState() const { return m_state; }

    /**
     * @brief Check if test is currently running
     */
    bool isRunning() const;

signals:
    void stateChanged(const TestRunState& state);
    void testCompleted(const TestResults& results);
    void testFailed(const FailureReason& reason);

private slots:
    void onCycleTick();

private:
    // Device references
    Infrastructure::Devices::IMotorDriveDevice* m_motor;
    Infrastructure::Devices::ITorqueSensorDevice* m_torque;
    Infrastructure::Devices::IEncoderDevice* m_encoder;
    Infrastructure::Devices::IBrakePowerDevice* m_brake;

    Infrastructure::Acquisition::AcquisitionScheduler* m_acquisitionScheduler;

    // Configuration
    TestRecipe m_recipe;
    QString m_stationName;
    int m_brakeChannel;  // Which channel to use for brake

    // Runtime state
    TestRunState m_state;
    QTimer* m_cycleTimer;
    QElapsedTimer m_testTimer;
    QElapsedTimer m_phaseTimer;

    // Magnet detection state
    bool m_lastAi1Level;
    bool m_magnetEventDetected;

    // Sampling buffers
    QVector<double> m_currentSamples;
    QVector<double> m_speedSamples;
    QVector<double> m_torqueSamples;
    static constexpr int MAX_SAMPLE_BUFFER_SIZE = 10000; // Max samples (~5 min at 33ms)

    // Impact test state
    int m_impactCycleCount;
    QVector<double> m_impactCurrentSamples;
    QVector<double> m_impactTorqueSamples;

// Lock detection state - improved state machine
enum class LockDetectionState {
Idle,        // Waiting for lock condition
WindowCheck, // Checking if speed is within window
HoldCheck,   // Holding for required duration
Locked       // Lock confirmed
};
LockDetectionState m_lockState;
QElapsedTimer m_lockTimer;
double m_lockReferenceAngle;
bool m_lockConditionMet = false;

    // Brake ramp state
    double m_currentBrakeCurrent;
    double m_currentBrakeVoltage = 0.0;
    quint64 m_settlingTargetMs = 0;

    // Phase time accumulator (total time across sub-states within a phase)
    qint64 m_phaseAccumulatedMs = 0;

    // Emergency stop flag (ADR-001: atomic for thread-safe interrupt)
    std::atomic<bool> m_emergencyStopRequested;

    // Physics validation (runtime monitoring)
    Infrastructure::Validation::PhysicsValidator::ValidationConfig m_validationConfig;
    Infrastructure::Validation::PhysicsViolationLogger* m_violationLogger;
    TelemetrySnapshot m_lastSnapshot;
    bool m_physicsValidationEnabled;

    // Simulation context (optional, for mock mode tick advancement)
    Infrastructure::Simulation::SimulationContext* m_simulationContext;

    // Phase management
    void transitionToPhase(TestPhase newPhase, TestSubState newSubState);
    void transitionToSubState(TestSubState newSubState);
    void updateProgress();

    // Telemetry acquisition
    bool acquireTelemetry(TelemetrySnapshot& snapshot);

    // Magnet event detection
    bool checkMagnetEvent(const TelemetrySnapshot& snapshot);

    // Phase handlers
    void handleImpactTestPhase();
    void handleHomingPhase();
    void handleIdleRunPhase();
    void handleAnglePositioningPhase();
    void handleLoadTestPhase();
    void handleReturnToZeroPhase();

    // Sub-state handlers
    void handleImpactForwardSpinup();
    void handleImpactForwardBrakeOn();
    void handleImpactForwardBrakeOff();
    void handleImpactReverseSpinup();
    void handleImpactReverseBrakeOn();
    void handleImpactReverseBrakeOff();
    void handleSeekingMagnet();
    void handleAdvancingToEncoderZero();
    void handleSpinupForward();
    void handleSampleForward();
    void handleSpinupReverse();
    void handleSampleReverse();
    void handleMoveToPosition1();
    void handleMoveToPosition2();
    void handleMoveBackToPosition1();
    void handleMoveToPosition3();
    void handleMoveBackToZero();
    void handleSpinupLoadForward();
    void handleRampBrakeForward();
    void handleConfirmLockForward();
    void handleSpinupLoadReverse();
    void handleRampBrakeReverse();
    void handleConfirmLockReverse();
    void handleReturnFinalZero();
    void handleSettlingForwardDelay();
    void handleSettlingReverseDelay();
    void handleSettlingPosition2Delay();
    void handleSettlingPosition1ReturnDelay();
    void handleSettlingPosition3Delay();
    void handleSettlingZeroDelay();
    void handleSettlingLoadForwardDelay();
    void handleSettlingLoadReverseDelay();

    // Judgment helpers
    void evaluateImpactResult(const QString& direction,
                              const QVector<ImpactCycleResult>& cycles,
                              ImpactDirectionResult& result,
                              double currentMin, double currentMax,
                              double torqueMin, double torqueMax);
    void evaluateIdleResults(const QString& direction,
                             const QVector<double>& currentSamples,
                             const QVector<double>& speedSamples,
                             IdleRunResult& result);
    void evaluateAngleResult(const QString& positionName,
                             double targetAngle,
                             double tolerance,
                             double measuredAngle);
    void evaluateLoadResult(const QString& direction,
                            double lockCurrent,
                            double lockTorque,
                            LoadTestResult& result);

    // Lock detection
    bool checkLockCondition(const TelemetrySnapshot& snapshot);

    // Failure handling
    void failTest(FailureCategory category, const QString& description);
    void completeTest();

    // Motor control helpers
    bool setMotorForward(double dutyCycle);
    bool setMotorReverse(double dutyCycle);
    bool stopMotor();
};

} // namespace Domain

#endif // GEARBOXTESTENGINE_H
