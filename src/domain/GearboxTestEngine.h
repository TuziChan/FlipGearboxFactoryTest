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

    /**
     * @brief Set test recipe
     */
    void setRecipe(const TestRecipe& recipe);
    void setBrakeChannel(int channel);

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

    // Configuration
    TestRecipe m_recipe;
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

    // Lock detection state
    QElapsedTimer m_lockConditionTimer;
    double m_lockStartAngle;
    bool m_lockConditionMet;

    // Brake ramp state
    double m_currentBrakeCurrent;

    // Phase management
    void transitionToPhase(TestPhase newPhase, TestSubState newSubState);
    void transitionToSubState(TestSubState newSubState);
    void updateProgress();

    // Telemetry acquisition
    bool acquireTelemetry(TelemetrySnapshot& snapshot);

    // Magnet event detection
    bool checkMagnetEvent(const TelemetrySnapshot& snapshot);

    // Phase handlers
    void handleHomingPhase();
    void handleIdleRunPhase();
    void handleAnglePositioningPhase();
    void handleLoadTestPhase();
    void handleReturnToZeroPhase();

    // Sub-state handlers
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

    // Judgment helpers
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
