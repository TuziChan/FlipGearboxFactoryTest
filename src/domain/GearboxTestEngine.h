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
    Infrastructure::Devices::IMotorDriveDevice* m_motor;
    Infrastructure::Devices::ITorqueSensorDevice* m_torque;
    Infrastructure::Devices::IEncoderDevice* m_encoder;
    Infrastructure::Devices::IBrakePowerDevice* m_brake;

    Infrastructure::Acquisition::AcquisitionScheduler* m_acquisitionScheduler;

    TestRecipe m_recipe;
    QString m_stationName;
    int m_brakeChannel;

    TestRunState m_state;
    QTimer* m_cycleTimer;
    QElapsedTimer m_testTimer;
    QElapsedTimer m_phaseTimer;

    bool m_lastAi1Level;
    bool m_magnetEventDetected;

    QVector<double> m_currentSamples;
    QVector<double> m_speedSamples;
    QVector<double> m_torqueSamples;
    static constexpr int MAX_SAMPLE_BUFFER_SIZE = 10000;

    int m_impactCycleCount;
    QVector<double> m_impactCurrentSamples;
    QVector<double> m_impactTorqueSamples;

    enum class LockDetectionState {
        Idle,
        WindowCheck,
        HoldCheck,
        Locked
    };
    LockDetectionState m_lockState;
    QElapsedTimer m_lockTimer;
    double m_lockReferenceAngle;
    bool m_lockConditionMet = false;

    double m_currentBrakeCurrent;
    double m_currentBrakeVoltage = 0.0;
    quint64 m_settlingTargetMs = 0;
    qint64 m_phaseAccumulatedMs = 0;

    std::atomic<bool> m_emergencyStopRequested;

    Infrastructure::Validation::PhysicsValidator::ValidationConfig m_validationConfig;
    Infrastructure::Validation::PhysicsViolationLogger* m_violationLogger;
    TelemetrySnapshot m_lastSnapshot;
    bool m_physicsValidationEnabled;

    void transitionToPhase(TestPhase newPhase, TestSubState newSubState);
    void transitionToSubState(TestSubState newSubState);
    void updateProgress();

    bool acquireTelemetry(TelemetrySnapshot& snapshot);
    bool checkMagnetEvent(const TelemetrySnapshot& snapshot);

    void handleImpactTestPhase();
    void handleHomingPhase();
    void handleIdleRunPhase();
    void handleAnglePositioningPhase();
    void handleLoadTestPhase();
    void handleReturnToZeroPhase();

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

    bool checkLockCondition(const TelemetrySnapshot& snapshot);
    void failTest(FailureCategory category, const QString& description);
    void completeTest();

    bool setMotorForward(double dutyCycle);
    bool setMotorReverse(double dutyCycle);
    bool stopMotor();
};

} // namespace Domain

#endif // GEARBOXTESTENGINE_H
