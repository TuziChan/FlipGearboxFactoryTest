#include "GearboxTestEngine.h"
#include "RecipeValidator.h"
#include <QDebug>
#include <QtMath>
#include <QThread>
#include <limits>

namespace Domain {

GearboxTestEngine::GearboxTestEngine(QObject* parent)
: QObject(parent)
, m_motor(nullptr)
, m_torque(nullptr)
, m_encoder(nullptr)
, m_brake(nullptr)
, m_acquisitionScheduler(nullptr)
, m_recipe()
, m_brakeChannel(1)
, m_state()
, m_cycleTimer(new QTimer(this))
, m_testTimer()
, m_phaseTimer()
, m_lastAi1Level(true)
, m_magnetEventDetected(false)
, m_currentSamples()
, m_speedSamples()
, m_lockState(LockDetectionState::Idle)
, m_lockTimer()
, m_lockReferenceAngle(0.0)
, m_currentBrakeCurrent(0.0)
{
// 33ms cycle time for 30Hz sampling (per correction document)
m_cycleTimer->setInterval(33);
connect(m_cycleTimer, &QTimer::timeout, this, &GearboxTestEngine::onCycleTick);
}

void GearboxTestEngine::setDevices(Infrastructure::Devices::IMotorDriveDevice* motor,
                                     Infrastructure::Devices::ITorqueSensorDevice* torque,
                                     Infrastructure::Devices::IEncoderDevice* encoder,
                                     Infrastructure::Devices::IBrakePowerDevice* brake) {
    m_motor = motor;
    m_torque = torque;
    m_encoder = encoder;
    m_brake = brake;
}

void GearboxTestEngine::setAcquisitionScheduler(Infrastructure::Acquisition::AcquisitionScheduler* scheduler) {
    m_acquisitionScheduler = scheduler;
}

void GearboxTestEngine::setRecipe(const TestRecipe& recipe) {
    // Validate recipe parameters
    QStringList errors;
    if (!RecipeValidator::validate(recipe, errors)) {
        qWarning() << "Recipe validation failed:";
        for (const QString& error : errors) {
            qWarning() << "  -" << error;
        }
        // Still set the recipe but log warnings
    }
    m_recipe = recipe;
}

void GearboxTestEngine::setBrakeChannel(int channel) {
    m_brakeChannel = qMax(1, channel);
}

bool GearboxTestEngine::startTest(const QString& serialNumber) {
    if (!m_motor || !m_torque || !m_encoder || !m_brake) {
        qWarning() << "Cannot start test: devices not set";
        return false;
    }

    // Initialize state
    m_state = TestRunState();
    m_state.results.serialNumber = serialNumber;
    m_state.results.recipeName = m_recipe.name;
    m_state.results.startTime = QDateTime::currentDateTime();

    // Reset detection state
    m_lastAi1Level = true;
    m_magnetEventDetected = false;
    m_lockConditionMet = false;

    // Start timers
    m_testTimer.start();
    m_phaseTimer.start();

    // Transition to homing phase
    transitionToPhase(TestPhase::PrepareAndHome, TestSubState::SeekingMagnet);

    // Start cycle timer
    m_cycleTimer->start();

    qDebug() << "Test started for SN:" << serialNumber;
    return true;
}

void GearboxTestEngine::emergencyStop() {
    m_cycleTimer->stop();
    stopMotor();
    
    if (m_brake) {
        m_brake->setOutputEnable(m_brakeChannel, false);
    }

    failTest(FailureCategory::Process, "Emergency stop triggered");
}

void GearboxTestEngine::reset() {
m_cycleTimer->stop();
stopMotor();

if (m_brake) {
m_brake->setOutputEnable(m_brakeChannel, false);
}

m_state = TestRunState();
// Reset lock detection state machine
m_lockState = LockDetectionState::Idle;
m_lockTimer.invalidate();
m_lockReferenceAngle = 0.0;
emit stateChanged(m_state);
}

void GearboxTestEngine::onCycleTick() {
    // Update elapsed times
    m_state.elapsedMs = m_testTimer.elapsed();
    m_state.phaseElapsedMs = m_phaseTimer.elapsed();

    // Acquire telemetry
    if (!acquireTelemetry(m_state.currentTelemetry)) {
        failTest(FailureCategory::Communication, "Failed to acquire telemetry");
        return;
    }

    // Check for magnet events
    if (checkMagnetEvent(m_state.currentTelemetry)) {
        m_magnetEventDetected = true;
    }

    // Dispatch to phase handler
    switch (m_state.phase) {
        case TestPhase::PrepareAndHome:
            handleHomingPhase();
            break;
        case TestPhase::IdleRun:
            handleIdleRunPhase();
            break;
        case TestPhase::AnglePositioning:
            handleAnglePositioningPhase();
            break;
        case TestPhase::LoadRampAndLock:
            handleLoadTestPhase();
            break;
        case TestPhase::ReturnToZero:
            handleReturnToZeroPhase();
            break;
        case TestPhase::Completed:
        case TestPhase::Failed:
            m_cycleTimer->stop();
            break;
        default:
            break;
    }

    // Update progress
    updateProgress();

    // Emit state change
    emit stateChanged(m_state);
}

// Helper methods implementation

bool GearboxTestEngine::acquireTelemetry(TelemetrySnapshot& snapshot) {
    if (m_acquisitionScheduler && m_acquisitionScheduler->isRunning()) {
        snapshot = m_acquisitionScheduler->snapshot();
        return true;
    }

    snapshot.timestamp = QDateTime::currentDateTime();

    if (!m_motor->readCurrent(snapshot.motorCurrentA)) {
        qWarning() << "Failed to read motor current:" << m_motor->lastError();
        return false;
    }

    if (!m_motor->readAI1Level(snapshot.aqmdAi1Level)) {
        qWarning() << "Failed to read AI1 level:" << m_motor->lastError();
        return false;
    }

    if (!m_torque->readAll(snapshot.dynTorqueNm, snapshot.dynSpeedRpm, snapshot.dynPowerW)) {
        qWarning() << "Failed to read torque sensor:" << m_torque->lastError();
        return false;
    }

    if (!m_encoder->readAngle(snapshot.encoderAngleDeg)) {
        qWarning() << "Failed to read encoder:" << m_encoder->lastError();
        return false;
    }

    if (!m_brake->readCurrent(m_brakeChannel, snapshot.brakeCurrentA)) {
        qWarning() << "Failed to read brake current:" << m_brake->lastError();
        return false;
    }

    m_brake->readVoltage(m_brakeChannel, snapshot.brakeVoltageV);
    m_brake->readPower(m_brakeChannel, snapshot.brakePowerW);

    return true;
}

bool GearboxTestEngine::checkMagnetEvent(const TelemetrySnapshot& snapshot) {
    // Detect falling edge: high -> low transition
    bool fallingEdge = m_lastAi1Level && !snapshot.aqmdAi1Level;
    m_lastAi1Level = snapshot.aqmdAi1Level;
    return fallingEdge;
}

void GearboxTestEngine::transitionToPhase(TestPhase newPhase, TestSubState newSubState) {
    m_state.phase = newPhase;
    m_state.subState = newSubState;
    m_phaseTimer.restart();
    m_phaseAccumulatedMs = 0;

    qDebug() << "Phase transition:" << m_state.phaseString() << "SubState:" << static_cast<int>(newSubState);
}

void GearboxTestEngine::transitionToSubState(TestSubState newSubState) {
    m_phaseAccumulatedMs += m_phaseTimer.elapsed();
    m_state.subState = newSubState;
    m_phaseTimer.restart();

    qDebug() << "SubState transition:" << static_cast<int>(newSubState);
}

void GearboxTestEngine::updateProgress() {
    double progress = 0.0;

    switch (m_state.phase) {
    case TestPhase::Idle:
        progress = 0.0;
        break;

    case TestPhase::PrepareAndHome: {
        // 0-15%
        double maxTime = qMax(1, m_recipe.homeTimeoutMs);
        switch (m_state.subState) {
        case TestSubState::SeekingMagnet: {
            double frac = qMin(1.0, static_cast<double>(m_phaseTimer.elapsed()) / maxTime);
            progress = frac * 10.0;
            break;
        }
        case TestSubState::AdvancingToEncoderZero: {
            double frac = qMin(1.0, static_cast<double>(m_phaseTimer.elapsed()) / maxTime);
            progress = 10.0 + frac * 4.0;
            break;
        }
        case TestSubState::HomeSettled:
            progress = 15.0;
            break;
        default:
            progress = 10.0;
            break;
        }
        break;
    }

    case TestPhase::IdleRun: {
        // 15-35%: proportional to estimated total idle run time
        double totalExpected = m_recipe.idleForwardSpinupMs + m_recipe.idleForwardSampleMs +
                               500.0 + m_recipe.idleReverseSpinupMs + m_recipe.idleReverseSampleMs + 500.0;
        qint64 totalPhaseMs = m_phaseAccumulatedMs + m_phaseTimer.elapsed();
        double phaseFrac = qMin(1.0, static_cast<double>(totalPhaseMs) / qMax(1.0, totalExpected));
        progress = 15.0 + phaseFrac * 20.0;
        break;
    }

    case TestPhase::AnglePositioning: {
        // 35-60%: 9 sub-states
        static const TestSubState states[] = {
            TestSubState::MoveToPosition1,
            TestSubState::MoveToPosition2,
            TestSubState::SettlingPosition2Delay,
            TestSubState::MoveBackToPosition1,
            TestSubState::SettlingPosition1ReturnDelay,
            TestSubState::MoveToPosition3,
            TestSubState::SettlingPosition3Delay,
            TestSubState::MoveBackToZero,
            TestSubState::SettlingZeroDelay,
        };
        int idx = -1;
        for (int i = 0; i < 9; i++) {
            if (m_state.subState == states[i]) { idx = i; break; }
        }
        if (idx >= 0) {
            double base = 35.0 + idx * (25.0 / 9.0);
            double subRange = 25.0 / 9.0;
            double refTime = qMax(1, m_recipe.angleTimeoutMs);
            if (m_state.subState == TestSubState::SettlingPosition2Delay ||
                m_state.subState == TestSubState::SettlingPosition1ReturnDelay ||
                m_state.subState == TestSubState::SettlingPosition3Delay ||
                m_state.subState == TestSubState::SettlingZeroDelay) {
                refTime = qMax(1, static_cast<int>(m_settlingTargetMs));
            }
            double frac = qMin(1.0, static_cast<double>(m_phaseTimer.elapsed()) / refTime);
            progress = base + frac * subRange;
        } else {
            progress = 35.0;
        }
        break;
    }

    case TestPhase::LoadRampAndLock: {
        // 60-85%: 6 sub-states
        static const TestSubState states[] = {
            TestSubState::SpinupLoadForward,
            TestSubState::RampBrakeForward,
            TestSubState::SettlingLoadForwardDelay,
            TestSubState::SpinupLoadReverse,
            TestSubState::RampBrakeReverse,
            TestSubState::SettlingLoadReverseDelay,
        };
        int idx = -1;
        for (int i = 0; i < 6; i++) {
            if (m_state.subState == states[i]) { idx = i; break; }
        }
        if (idx >= 0) {
            double base = 60.0 + idx * (25.0 / 6.0);
            double subRange = 25.0 / 6.0;
            double refTime = qMax(1, m_recipe.loadSpinupMs);
            if (m_state.subState == TestSubState::RampBrakeForward ||
                m_state.subState == TestSubState::RampBrakeReverse) {
                refTime = qMax(1, m_recipe.loadRampMs);
            } else if (m_state.subState == TestSubState::SettlingLoadForwardDelay ||
                       m_state.subState == TestSubState::SettlingLoadReverseDelay) {
                refTime = qMax(1, static_cast<int>(m_settlingTargetMs));
            }
            double frac = qMin(1.0, static_cast<double>(m_phaseTimer.elapsed()) / refTime);
            progress = base + frac * subRange;
        } else {
            progress = 60.0;
        }
        break;
    }

    case TestPhase::ReturnToZero: {
        // 85-100%
        double refTime = qMax(1, m_recipe.returnZeroTimeoutMs);
        qint64 totalPhaseMs = m_phaseAccumulatedMs + m_phaseTimer.elapsed();
        double frac = qMin(1.0, static_cast<double>(totalPhaseMs) / refTime);
        progress = 85.0 + frac * 15.0;
        break;
    }

    case TestPhase::Completed:
    case TestPhase::Failed:
        progress = 100.0;
        break;
    }

    m_state.progressPercent = qBound(0, static_cast<int>(progress), 100);
}

bool GearboxTestEngine::setMotorForward(double dutyCycle) {
    m_state.currentDirection = MotorDirection::Forward;
    return m_motor->setMotor(Infrastructure::Devices::IMotorDriveDevice::Direction::Forward, dutyCycle);
}

bool GearboxTestEngine::setMotorReverse(double dutyCycle) {
    m_state.currentDirection = MotorDirection::Reverse;
    return m_motor->setMotor(Infrastructure::Devices::IMotorDriveDevice::Direction::Reverse, dutyCycle);
}

bool GearboxTestEngine::stopMotor() {
    m_state.currentDirection = MotorDirection::Stopped;
    return m_motor->brake();
}

void GearboxTestEngine::failTest(FailureCategory category, const QString& description) {
    qCritical() << "========================================";
    qCritical() << "TEST FAILURE DETECTED";
    qCritical() << "========================================";
    qCritical() << "Category:" << category;
    qCritical() << "Description:" << description;
    qCritical() << "Phase:" << static_cast<int>(m_state.phase);
    qCritical() << "SubState:" << static_cast<int>(m_state.subState);
    qCritical() << "Elapsed time:" << m_state.elapsedMs << "ms";
    qCritical() << "========================================";
    
    // Phase 1: Stop cycle timer immediately
    qDebug() << "Phase 1: Stopping cycle timer...";
    m_cycleTimer->stop();
    
    // Phase 2: Emergency stop motor
    qDebug() << "Phase 2: Emergency stopping motor...";
    if (!stopMotor()) {
        qCritical() << "  [WARNING] Failed to stop motor during failure handling!";
        // Try again with direct brake command
        if (m_motor) {
            m_motor->brake();
        }
    } else {
        qDebug() << "  [OK] Motor stopped";
    }
    
    // Phase 3: Disable brake power supply
    qDebug() << "Phase 3: Disabling brake power supply...";
    if (m_brake) {
        if (!m_brake->setOutputEnable(m_brakeChannel, false)) {
            qCritical() << "  [WARNING] Failed to disable brake output during failure handling!";
            // Try multiple times for safety
            for (int retry = 0; retry < 3; ++retry) {
                QThread::msleep(50);
                if (m_brake->setOutputEnable(m_brakeChannel, false)) {
                    qDebug() << "  [OK] Brake disabled on retry" << (retry + 1);
                    break;
                }
            }
        } else {
            qDebug() << "  [OK] Brake power supply disabled";
        }
    }
    
    // Phase 4: Clear sample buffers
    qDebug() << "Phase 4: Clearing sample buffers...";
    m_currentSamples.clear();
    m_speedSamples.clear();
    m_torqueSamples.clear();
    qDebug() << "  [OK] Sample buffers cleared";
    
    // Phase 5: Reset state flags
    qDebug() << "Phase 5: Resetting state flags...";
    m_magnetEventDetected = false;
    m_lockConditionMet = false;
    m_currentBrakeCurrent = 0.0;
    m_currentBrakeVoltage = 0.0;
    qDebug() << "  [OK] State flags reset";

    // Phase 6: Update test state
    qDebug() << "Phase 6: Updating test state...";
    m_state.phase = TestPhase::Failed;
    m_state.subState = TestSubState::TestFailed;
    m_state.currentDirection = MotorDirection::Stopped;
    m_state.results.overallPassed = false;
    m_state.results.failure = FailureReason(category, description);
    m_state.results.endTime = QDateTime::currentDateTime();
    m_state.statusMessage = QString("FAILED: %1").arg(description);

    QString categoryStr;
    switch (category) {
        case FailureCategory::None:
            categoryStr = "None";
            break;
        case FailureCategory::Communication:
            categoryStr = "Communication";
            break;
        case FailureCategory::Process:
            categoryStr = "Process";
            break;
        case FailureCategory::Judgment:
            categoryStr = "Judgment";
            break;
    }
    
    qCritical() << "========================================";
    qCritical() << "TEST FAILED - CLEANUP COMPLETE";
    qCritical() << "Category:" << categoryStr;
    qCritical() << "Reason:" << description;
    qCritical() << "System is now in safe state";
    qCritical() << "========================================";

    emit testFailed(m_state.results.failure);
    emit stateChanged(m_state);
}

void GearboxTestEngine::completeTest() {
    m_cycleTimer->stop();
    stopMotor();
    
    if (m_brake) {
        m_brake->setOutputEnable(m_brakeChannel, false);
    }

    m_state.phase = TestPhase::Completed;
    m_state.subState = TestSubState::TestCompleted;
    m_state.results.endTime = QDateTime::currentDateTime();
    
    // Determine overall pass/fail
    m_state.results.overallPassed = 
        m_state.results.idleForward.overallPassed &&
        m_state.results.idleReverse.overallPassed &&
        m_state.results.loadForward.overallPassed &&
        m_state.results.loadReverse.overallPassed;
    
    for (const auto& angleResult : std::as_const(m_state.results.angleResults)) {
        if (!angleResult.passed) {
            m_state.results.overallPassed = false;
            break;
        }
    }

    if (m_state.results.overallPassed) {
        m_state.statusMessage = "Test PASSED";
    } else {
        m_state.statusMessage = "Test FAILED (judgment)";
        m_state.results.failure = FailureReason(FailureCategory::Judgment, "One or more measurements out of spec");
    }

    qDebug() << "Test completed:" << (m_state.results.overallPassed ? "PASSED" : "FAILED");

    emit testCompleted(m_state.results);
    emit stateChanged(m_state);
}

// Phase handlers implementation

void GearboxTestEngine::handleHomingPhase() {
    switch (m_state.subState) {
        case TestSubState::SeekingMagnet:
            handleSeekingMagnet();
            break;
        case TestSubState::AdvancingToEncoderZero:
            handleAdvancingToEncoderZero();
            break;
        case TestSubState::HomeSettled:
            // Homing complete, move to idle run
            transitionToPhase(TestPhase::IdleRun, TestSubState::SpinupForward);
            break;
        default:
            break;
    }
}

void GearboxTestEngine::handleSeekingMagnet() {
    // Start motor if not already running
    if (m_state.currentDirection != MotorDirection::Forward) {
        if (!setMotorForward(m_recipe.homeDutyCycle)) {
            failTest(FailureCategory::Communication, "Failed to start motor for homing");
            return;
        }
        m_state.statusMessage = "Seeking magnet...";
    }

    // Check for magnet event
    if (m_magnetEventDetected) {
        qDebug() << "Magnet detected at angle:" << m_state.currentTelemetry.encoderAngleDeg;
        m_magnetEventDetected = false;
        transitionToSubState(TestSubState::AdvancingToEncoderZero);
        return;
    }

    // Check timeout
    if (m_state.phaseElapsedMs > m_recipe.homeTimeoutMs) {
        failTest(FailureCategory::Process, "Homing timeout: magnet not detected");
    }
}

void GearboxTestEngine::handleAdvancingToEncoderZero() {
    // Continue forward to encoder zero point
    double currentAngle = m_state.currentTelemetry.encoderAngleDeg;
    double targetAngle = m_recipe.encoderZeroAngleDeg;
    
    // Check if we've reached the target
    double angleDiff = qAbs(currentAngle - targetAngle);
    if (angleDiff < 0.5) {  // Within 0.5 degree
        stopMotor();
        m_state.results.homingCompleted = true;
        m_state.results.finalEncoderZeroDeg = currentAngle;
        m_state.statusMessage = "Homing complete";
        transitionToSubState(TestSubState::HomeSettled);
        qDebug() << "Homing complete at angle:" << currentAngle;
        return;
    }

    // Check timeout
    if (m_state.phaseElapsedMs > m_recipe.homeTimeoutMs) {
        failTest(FailureCategory::Process, "Homing timeout: failed to reach encoder zero");
    }
}

void GearboxTestEngine::handleIdleRunPhase() {
    // Phase-level timeout check (total time across sub-states)
    qint64 totalPhaseMs = m_phaseAccumulatedMs + m_state.phaseElapsedMs;
    if (totalPhaseMs > m_recipe.idleTimeoutMs) {
        failTest(FailureCategory::Process, "Idle run phase timeout");
        return;
    }

    switch (m_state.subState) {
        case TestSubState::SpinupForward:
            handleSpinupForward();
            break;
        case TestSubState::SampleForward:
            handleSampleForward();
            break;
        case TestSubState::SpinupReverse:
            handleSpinupReverse();
            break;
        case TestSubState::SampleReverse:
            handleSampleReverse();
            break;
        case TestSubState::SettlingForwardDelay:
            handleSettlingForwardDelay();
            break;
        case TestSubState::SettlingReverseDelay:
            handleSettlingReverseDelay();
            break;
        default:
            break;
    }
}

void GearboxTestEngine::handleSpinupForward() {
    // Start motor if not running
    if (m_state.currentDirection != MotorDirection::Forward) {
        if (!setMotorForward(m_recipe.idleDutyCycle)) {
            failTest(FailureCategory::Communication, "Failed to start motor for idle forward");
            return;
        }
        m_currentSamples.clear();
        m_speedSamples.clear();
        m_state.statusMessage = "Idle forward spinup...";
    }

    // Wait for spinup time
    if (m_state.phaseElapsedMs >= m_recipe.idleForwardSpinupMs) {
        transitionToSubState(TestSubState::SampleForward);
    }
}

void GearboxTestEngine::handleSampleForward() {
    m_state.statusMessage = "Sampling idle forward...";
    
    // Collect samples with buffer overflow protection
    if (m_currentSamples.size() < MAX_SAMPLE_BUFFER_SIZE) {
        m_currentSamples.append(m_state.currentTelemetry.motorCurrentA);
        m_speedSamples.append(m_state.currentTelemetry.dynSpeedRpm);
    } else {
        qWarning() << "Sample buffer overflow detected, stopping sampling";
        failTest(FailureCategory::Process, "Sample buffer overflow in forward sampling");
        return;
    }

    // Check if sampling window complete
    if (m_state.phaseElapsedMs >= m_recipe.idleForwardSampleMs) {
        // Evaluate results
        evaluateIdleResults("Forward", m_currentSamples, m_speedSamples, m_state.results.idleForward);
        
        // Move to reverse
        stopMotor();
        m_settlingTargetMs = 500;
        transitionToSubState(TestSubState::SettlingForwardDelay);
    }
}

void GearboxTestEngine::handleSpinupReverse() {
    // Start motor if not running
    if (m_state.currentDirection != MotorDirection::Reverse) {
        if (!setMotorReverse(m_recipe.idleDutyCycle)) {
            failTest(FailureCategory::Communication, "Failed to start motor for idle reverse");
            return;
        }
        m_currentSamples.clear();
        m_speedSamples.clear();
        m_state.statusMessage = "Idle reverse spinup...";
    }

    // Wait for spinup time
    if (m_state.phaseElapsedMs >= m_recipe.idleReverseSpinupMs) {
        transitionToSubState(TestSubState::SampleReverse);
    }
}

void GearboxTestEngine::handleSampleReverse() {
    m_state.statusMessage = "Sampling idle reverse...";
    
    // Collect samples with buffer overflow protection
    if (m_currentSamples.size() < MAX_SAMPLE_BUFFER_SIZE) {
        m_currentSamples.append(m_state.currentTelemetry.motorCurrentA);
        m_speedSamples.append(qAbs(m_state.currentTelemetry.dynSpeedRpm));  // Absolute value for reverse
    } else {
        qWarning() << "Sample buffer overflow detected, stopping sampling";
        failTest(FailureCategory::Process, "Sample buffer overflow in reverse sampling");
        return;
    }

    // Check if sampling window complete
    if (m_state.phaseElapsedMs >= m_recipe.idleReverseSampleMs) {
        // Evaluate results
        evaluateIdleResults("Reverse", m_currentSamples, m_speedSamples, m_state.results.idleReverse);
        
        // Move to angle positioning
        stopMotor();
        m_settlingTargetMs = 500;
        transitionToSubState(TestSubState::SettlingReverseDelay);
    }
}

void GearboxTestEngine::handleAnglePositioningPhase() {
    switch (m_state.subState) {
        case TestSubState::MoveToPosition1:
            handleMoveToPosition1();
            break;
        case TestSubState::MoveToPosition2:
            handleMoveToPosition2();
            break;
        case TestSubState::MoveBackToPosition1:
            handleMoveBackToPosition1();
            break;
        case TestSubState::MoveToPosition3:
            handleMoveToPosition3();
            break;
        case TestSubState::MoveBackToZero:
            handleMoveBackToZero();
            break;
        case TestSubState::SettlingPosition2Delay:
            handleSettlingPosition2Delay();
            break;
        case TestSubState::SettlingPosition1ReturnDelay:
            handleSettlingPosition1ReturnDelay();
            break;
        case TestSubState::SettlingPosition3Delay:
            handleSettlingPosition3Delay();
            break;
        case TestSubState::SettlingZeroDelay:
            handleSettlingZeroDelay();
            break;
        default:
            break;
    }
}

void GearboxTestEngine::handleMoveToPosition1() {
    // Start motor if not running
    if (m_state.currentDirection != MotorDirection::Forward) {
        if (!setMotorForward(m_recipe.angleTestDutyCycle)) {
            failTest(FailureCategory::Communication, "Failed to start motor for position 1");
            return;
        }
        m_magnetEventDetected = false;
        m_state.statusMessage = "Moving to position 1...";
    }

    // Wait for magnet event
    if (m_magnetEventDetected) {
        double measuredAngle = m_state.currentTelemetry.encoderAngleDeg;
        evaluateAngleResult("Position 1 (first)", m_recipe.position1TargetDeg, 
                           m_recipe.position1ToleranceDeg, measuredAngle);
        m_magnetEventDetected = false;
        transitionToSubState(TestSubState::MoveToPosition2);
        return;
    }

    // Check timeout
    if (m_state.phaseElapsedMs > m_recipe.angleTimeoutMs) {
        failTest(FailureCategory::Process, "Timeout waiting for position 1");
    }
}

void GearboxTestEngine::handleMoveToPosition2() {
    // Continue forward to position 2
    if (m_state.currentDirection != MotorDirection::Forward) {
        if (!setMotorForward(m_recipe.angleTestDutyCycle)) {
            failTest(FailureCategory::Communication, "Failed to continue to position 2");
            return;
        }
        m_magnetEventDetected = false;
        m_state.statusMessage = "Moving to position 2...";
    }

    // Wait for magnet event
    if (m_magnetEventDetected) {
        double measuredAngle = m_state.currentTelemetry.encoderAngleDeg;
        evaluateAngleResult("Position 2", m_recipe.position2TargetDeg, 
                           m_recipe.position2ToleranceDeg, measuredAngle);
        m_magnetEventDetected = false;
        stopMotor();
        m_settlingTargetMs = 200;
        transitionToSubState(TestSubState::SettlingPosition2Delay);
        return;
    }

    // Check timeout
    if (m_state.phaseElapsedMs > m_recipe.angleTimeoutMs) {
        failTest(FailureCategory::Process, "Timeout waiting for position 2");
    }
}

void GearboxTestEngine::handleMoveBackToPosition1() {
    // Reverse back to position 1
    if (m_state.currentDirection != MotorDirection::Reverse) {
        if (!setMotorReverse(m_recipe.angleTestDutyCycle)) {
            failTest(FailureCategory::Communication, "Failed to reverse to position 1");
            return;
        }
        m_magnetEventDetected = false;
        m_state.statusMessage = "Returning to position 1...";
    }

    // Wait for magnet event
    if (m_magnetEventDetected) {
        double measuredAngle = m_state.currentTelemetry.encoderAngleDeg;
        evaluateAngleResult("Position 1 (return)", m_recipe.position1TargetDeg, 
                           m_recipe.position1ToleranceDeg, measuredAngle);
        m_magnetEventDetected = false;
        stopMotor();
        m_settlingTargetMs = 200;
        transitionToSubState(TestSubState::SettlingPosition1ReturnDelay);
        return;
    }

    // Check timeout
    if (m_state.phaseElapsedMs > m_recipe.angleTimeoutMs) {
        failTest(FailureCategory::Process, "Timeout returning to position 1");
    }
}

void GearboxTestEngine::handleMoveToPosition3() {
    // Forward to position 3
    if (m_state.currentDirection != MotorDirection::Forward) {
        if (!setMotorForward(m_recipe.angleTestDutyCycle)) {
            failTest(FailureCategory::Communication, "Failed to move to position 3");
            return;
        }
        m_magnetEventDetected = false;
        m_state.statusMessage = "Moving to position 3...";
    }

    // Wait for magnet event
    if (m_magnetEventDetected) {
        double measuredAngle = m_state.currentTelemetry.encoderAngleDeg;
        evaluateAngleResult("Position 3", m_recipe.position3TargetDeg, 
                           m_recipe.position3ToleranceDeg, measuredAngle);
        m_magnetEventDetected = false;
        stopMotor();
        m_settlingTargetMs = 200;
        transitionToSubState(TestSubState::SettlingPosition3Delay);
        return;
    }

    // Check timeout
    if (m_state.phaseElapsedMs > m_recipe.angleTimeoutMs) {
        failTest(FailureCategory::Process, "Timeout waiting for position 3");
    }
}

void GearboxTestEngine::handleMoveBackToZero() {
    // Return to encoder zero
    if (m_state.currentDirection != MotorDirection::Reverse) {
        if (!setMotorReverse(m_recipe.angleTestDutyCycle)) {
            failTest(FailureCategory::Communication, "Failed to return to zero");
            return;
        }
        m_state.statusMessage = "Returning to zero...";
    }

    double currentAngle = m_state.currentTelemetry.encoderAngleDeg;
    double targetAngle = m_recipe.encoderZeroAngleDeg;
    
    // Check if we've reached zero
    double angleDiff = qAbs(currentAngle - targetAngle);
    if (angleDiff < m_recipe.returnZeroToleranceDeg) {
        stopMotor();
        // Record zero position angle result (5th measurement)
        evaluateAngleResult("Zero", m_recipe.encoderZeroAngleDeg,
                           m_recipe.returnZeroToleranceDeg, currentAngle);
        m_settlingTargetMs = 500;
        transitionToSubState(TestSubState::SettlingZeroDelay);
        return;
    }

    // Check timeout
    if (m_state.phaseElapsedMs > m_recipe.angleTimeoutMs) {
        failTest(FailureCategory::Process, "Timeout returning to zero after angle test");
    }
}

void GearboxTestEngine::handleLoadTestPhase() {
    // Phase-level timeout check (total time across sub-states)
    qint64 totalPhaseMs = m_phaseAccumulatedMs + m_state.phaseElapsedMs;
    if (totalPhaseMs > m_recipe.loadTimeoutMs) {
        failTest(FailureCategory::Process, "Load test phase timeout");
        return;
    }

    switch (m_state.subState) {
        case TestSubState::SpinupLoadForward:
            handleSpinupLoadForward();
            break;
        case TestSubState::RampBrakeForward:
            handleRampBrakeForward();
            break;
        case TestSubState::ConfirmLockForward:
            handleConfirmLockForward();
            break;
        case TestSubState::SpinupLoadReverse:
            handleSpinupLoadReverse();
            break;
        case TestSubState::RampBrakeReverse:
            handleRampBrakeReverse();
            break;
        case TestSubState::ConfirmLockReverse:
            handleConfirmLockReverse();
            break;
        case TestSubState::SettlingLoadForwardDelay:
            handleSettlingLoadForwardDelay();
            break;
        case TestSubState::SettlingLoadReverseDelay:
            handleSettlingLoadReverseDelay();
            break;
        default:
            break;
    }
}

void GearboxTestEngine::handleSpinupLoadForward() {
    // Start motor if not running
    if (m_state.currentDirection != MotorDirection::Forward) {
        if (!setMotorForward(m_recipe.loadDutyCycle)) {
            failTest(FailureCategory::Communication, "Failed to start motor for load forward");
            return;
        }
        m_state.statusMessage = "Load forward spinup...";
    }

    // Wait for spinup time
    if (m_state.phaseElapsedMs >= m_recipe.loadSpinupMs) {
        // Set brake mode before enabling
        if (m_recipe.brakeMode == "CV") {
            if (!m_brake->setBrakeMode(m_brakeChannel, "CV")) {
                failTest(FailureCategory::Communication, "Failed to set brake mode to CV");
                return;
            }
            m_currentBrakeVoltage = m_recipe.brakeRampStartVoltage;
        } else {
            if (!m_brake->setBrakeMode(m_brakeChannel, "CC")) {
                failTest(FailureCategory::Communication, "Failed to set brake mode to CC");
                return;
            }
        }
        // Enable brake and start ramping
        if (!m_brake->setOutputEnable(m_brakeChannel, true)) {
            failTest(FailureCategory::Communication, "Failed to enable brake");
            return;
        }
        m_currentBrakeCurrent = m_recipe.brakeRampStartCurrentA;
        m_lockConditionMet = false;
        transitionToSubState(TestSubState::RampBrakeForward);
    }
}

void GearboxTestEngine::handleRampBrakeForward() {
    m_state.statusMessage = "Ramping brake forward...";
    
    // Calculate current brake current based on elapsed time
    double rampProgress = static_cast<double>(m_state.phaseElapsedMs) / m_recipe.loadRampMs;
    rampProgress = qBound(0.0, rampProgress, 1.0);
    
    if (m_recipe.brakeMode == "CV") {
        m_currentBrakeVoltage = m_recipe.brakeRampStartVoltage +
                                (m_recipe.brakeRampEndVoltage - m_recipe.brakeRampStartVoltage) * rampProgress;
        m_currentBrakeVoltage = qBound(0.0, m_currentBrakeVoltage, 24.0);
        if (!m_brake->setVoltage(m_brakeChannel, m_currentBrakeVoltage)) {
            failTest(FailureCategory::Communication, "Failed to set brake voltage");
            return;
        }
    } else {
        m_currentBrakeCurrent = m_recipe.brakeRampStartCurrentA +
                                (m_recipe.brakeRampEndCurrentA - m_recipe.brakeRampStartCurrentA) * rampProgress;
        m_currentBrakeCurrent = qBound(0.0, m_currentBrakeCurrent, 5.0);
        if (!m_brake->setCurrent(m_brakeChannel, m_currentBrakeCurrent)) {
            failTest(FailureCategory::Communication, "Failed to set brake current");
            return;
        }
    }

    // Check for lock condition
    if (checkLockCondition(m_state.currentTelemetry)) {
        // Lock achieved
        double lockCurrent = m_state.currentTelemetry.brakeCurrentA;
        double lockTorque = m_state.currentTelemetry.dynTorqueNm;
        
        m_state.results.loadForward.lockAchieved = true;
        evaluateLoadResult("Forward", lockCurrent, lockTorque, m_state.results.loadForward);
        
        // Disable brake and stop motor
        m_brake->setOutputEnable(m_brakeChannel, false);
        stopMotor();
        m_settlingTargetMs = 500;
        
        transitionToSubState(TestSubState::SettlingLoadForwardDelay);
        return;
    }

    // Check if ramp time exceeded without lock
    if (m_state.phaseElapsedMs > m_recipe.loadRampMs) {
        m_brake->setOutputEnable(m_brakeChannel, false);
        failTest(FailureCategory::Process, "Load forward: lock not achieved within ramp time");
    }
}

void GearboxTestEngine::handleConfirmLockForward() {
    // This state is currently unused as we confirm lock in RampBrakeForward
    // Kept for potential future use
}

void GearboxTestEngine::handleSpinupLoadReverse() {
    // Start motor if not running
    if (m_state.currentDirection != MotorDirection::Reverse) {
        if (!setMotorReverse(m_recipe.loadDutyCycle)) {
            failTest(FailureCategory::Communication, "Failed to start motor for load reverse");
            return;
        }
        m_state.statusMessage = "Load reverse spinup...";
    }

    // Wait for spinup time
    if (m_state.phaseElapsedMs >= m_recipe.loadSpinupMs) {
        // Set brake mode before enabling
        if (m_recipe.brakeMode == "CV") {
            if (!m_brake->setBrakeMode(m_brakeChannel, "CV")) {
                failTest(FailureCategory::Communication, "Failed to set brake mode to CV");
                return;
            }
            m_currentBrakeVoltage = m_recipe.brakeRampStartVoltage;
        } else {
            if (!m_brake->setBrakeMode(m_brakeChannel, "CC")) {
                failTest(FailureCategory::Communication, "Failed to set brake mode to CC");
                return;
            }
        }
        // Enable brake and start ramping
        if (!m_brake->setOutputEnable(m_brakeChannel, true)) {
            failTest(FailureCategory::Communication, "Failed to enable brake");
            return;
        }
        m_currentBrakeCurrent = m_recipe.brakeRampStartCurrentA;
        m_lockConditionMet = false;
        transitionToSubState(TestSubState::RampBrakeReverse);
    }
}

void GearboxTestEngine::handleRampBrakeReverse() {
    m_state.statusMessage = "Ramping brake reverse...";
    
    // Calculate current brake current based on elapsed time
    double rampProgress = static_cast<double>(m_state.phaseElapsedMs) / m_recipe.loadRampMs;
    rampProgress = qBound(0.0, rampProgress, 1.0);
    
    if (m_recipe.brakeMode == "CV") {
        m_currentBrakeVoltage = m_recipe.brakeRampStartVoltage +
                                (m_recipe.brakeRampEndVoltage - m_recipe.brakeRampStartVoltage) * rampProgress;
        m_currentBrakeVoltage = qBound(0.0, m_currentBrakeVoltage, 24.0);
        if (!m_brake->setVoltage(m_brakeChannel, m_currentBrakeVoltage)) {
            failTest(FailureCategory::Communication, "Failed to set brake voltage");
            return;
        }
    } else {
        m_currentBrakeCurrent = m_recipe.brakeRampStartCurrentA +
                                (m_recipe.brakeRampEndCurrentA - m_recipe.brakeRampStartCurrentA) * rampProgress;
        m_currentBrakeCurrent = qBound(0.0, m_currentBrakeCurrent, 5.0);
        if (!m_brake->setCurrent(m_brakeChannel, m_currentBrakeCurrent)) {
            failTest(FailureCategory::Communication, "Failed to set brake current");
            return;
        }
    }

    // Check for lock condition
    if (checkLockCondition(m_state.currentTelemetry)) {
        // Lock achieved
        double lockCurrent = m_state.currentTelemetry.brakeCurrentA;
        double lockTorque = qAbs(m_state.currentTelemetry.dynTorqueNm);  // Absolute value for reverse
        
        m_state.results.loadReverse.lockAchieved = true;
        evaluateLoadResult("Reverse", lockCurrent, lockTorque, m_state.results.loadReverse);
        
        // Disable brake and stop motor
        m_brake->setOutputEnable(m_brakeChannel, false);
        stopMotor();
        m_settlingTargetMs = 500;
        
        transitionToSubState(TestSubState::SettlingLoadReverseDelay);
        return;
    }

    // Check if ramp time exceeded without lock
    if (m_state.phaseElapsedMs > m_recipe.loadRampMs) {
        m_brake->setOutputEnable(m_brakeChannel, false);
        failTest(FailureCategory::Process, "Load reverse: lock not achieved within ramp time");
    }
}

void GearboxTestEngine::handleConfirmLockReverse() {
    // This state is currently unused as we confirm lock in RampBrakeReverse
    // Kept for potential future use
}

void GearboxTestEngine::handleReturnToZeroPhase() {
    qint64 totalPhaseMs = m_phaseAccumulatedMs + m_state.phaseElapsedMs;
    if (totalPhaseMs > m_recipe.returnZeroTimeoutMs) {
        failTest(FailureCategory::Process, "Return to zero phase timeout");
        return;
    }

    switch (m_state.subState) {
    case TestSubState::ReturnFinalZero:
        handleReturnFinalZero();
        break;
    case TestSubState::FinalZeroSettled:
        completeTest();
        break;
    default:
        break;
    }
}

void GearboxTestEngine::handleReturnFinalZero() {
    // Start motor if not running
    if (m_state.currentDirection != MotorDirection::Forward) {
        if (!setMotorForward(m_recipe.angleTestDutyCycle)) {
            failTest(FailureCategory::Communication, "Failed to start motor for final return");
            return;
        }
        m_state.statusMessage = "Returning to zero...";
    }

    double currentAngle = m_state.currentTelemetry.encoderAngleDeg;
    double targetAngle = m_recipe.encoderZeroAngleDeg;

    // Check if we've reached zero
    double angleDiff = qAbs(currentAngle - targetAngle);
    if (angleDiff < m_recipe.returnZeroToleranceDeg) {
        stopMotor();
        m_state.statusMessage = "Final zero settled";
        transitionToSubState(TestSubState::FinalZeroSettled);
        return;
    }
}

// Evaluation and judgment helpers

void GearboxTestEngine::evaluateIdleResults(const QString& direction, 
                                             const QVector<double>& currentSamples,
                                             const QVector<double>& speedSamples,
                                             IdleRunResult& result) {
    result.direction = direction;
    
    if (currentSamples.isEmpty() || speedSamples.isEmpty()) {
        qWarning() << "Insufficient samples for idle" << direction;
        result.overallPassed = false;
        return;
    }

    // Calculate averages
    double currentSum = 0.0;
    double speedSum = 0.0;
    result.currentMax = currentSamples[0];
    result.speedMax = speedSamples[0];
    
    for (double current : currentSamples) {
        currentSum += current;
        if (current > result.currentMax) {
            result.currentMax = current;
        }
    }
    
    for (double speed : speedSamples) {
        speedSum += speed;
        if (speed > result.speedMax) {
            result.speedMax = speed;
        }
    }
    
    result.currentAvg = currentSum / currentSamples.size();
    result.speedAvg = speedSum / speedSamples.size();

    // Get limits based on direction
    double currentAvgMin, currentAvgMax, currentMaxMin, currentMaxMax;
    double speedAvgMin, speedAvgMax, speedMaxMin, speedMaxMax;
    
    if (direction == "Forward") {
        currentAvgMin = m_recipe.idleForwardCurrentAvgMin;
        currentAvgMax = m_recipe.idleForwardCurrentAvgMax;
        currentMaxMin = m_recipe.idleForwardCurrentMaxMin;
        currentMaxMax = m_recipe.idleForwardCurrentMaxMax;
        speedAvgMin = m_recipe.idleForwardSpeedAvgMin;
        speedAvgMax = m_recipe.idleForwardSpeedAvgMax;
        speedMaxMin = m_recipe.idleForwardSpeedMaxMin;
        speedMaxMax = m_recipe.idleForwardSpeedMaxMax;
    } else {
        currentAvgMin = m_recipe.idleReverseCurrentAvgMin;
        currentAvgMax = m_recipe.idleReverseCurrentAvgMax;
        currentMaxMin = m_recipe.idleReverseCurrentMaxMin;
        currentMaxMax = m_recipe.idleReverseCurrentMaxMax;
        speedAvgMin = m_recipe.idleReverseSpeedAvgMin;
        speedAvgMax = m_recipe.idleReverseSpeedAvgMax;
        speedMaxMin = m_recipe.idleReverseSpeedMaxMin;
        speedMaxMax = m_recipe.idleReverseSpeedMaxMax;
    }

    // Evaluate each metric
    result.currentAvgPassed = (result.currentAvg >= currentAvgMin) && (result.currentAvg <= currentAvgMax);
    result.currentMaxPassed = (result.currentMax >= currentMaxMin) && (result.currentMax <= currentMaxMax);
    result.speedAvgPassed = (result.speedAvg >= speedAvgMin) && (result.speedAvg <= speedAvgMax);
    result.speedMaxPassed = (result.speedMax >= speedMaxMin) && (result.speedMax <= speedMaxMax);
    
    result.overallPassed = result.currentAvgPassed && result.currentMaxPassed && 
                           result.speedAvgPassed && result.speedMaxPassed;

    qDebug() << "Idle" << direction << "results:"
             << "Current avg:" << result.currentAvg << (result.currentAvgPassed ? "PASS" : "FAIL")
             << "Current max:" << result.currentMax << (result.currentMaxPassed ? "PASS" : "FAIL")
             << "Speed avg:" << result.speedAvg << (result.speedAvgPassed ? "PASS" : "FAIL")
             << "Speed max:" << result.speedMax << (result.speedMaxPassed ? "PASS" : "FAIL");
}

void GearboxTestEngine::evaluateAngleResult(const QString& positionName,
                                             double targetAngle,
                                             double tolerance,
                                             double measuredAngle) {
    AngleResult result;
    result.positionName = positionName;
    result.targetAngleDeg = targetAngle;
    result.measuredAngleDeg = measuredAngle;
    result.deviationDeg = measuredAngle - targetAngle;
    result.toleranceDeg = tolerance;
    result.passed = qAbs(result.deviationDeg) <= tolerance;
    
    m_state.results.angleResults.append(result);
    
    qDebug() << "Angle result:" << positionName 
             << "Target:" << targetAngle 
             << "Measured:" << measuredAngle 
             << "Deviation:" << result.deviationDeg
             << (result.passed ? "PASS" : "FAIL");
}

void GearboxTestEngine::evaluateLoadResult(const QString& direction,
                                            double lockCurrent,
                                            double lockTorque,
                                            LoadTestResult& result) {
    result.direction = direction;
    result.lockCurrentA = lockCurrent;
    result.lockTorqueNm = lockTorque;
    
    // Get limits based on direction
    double currentMin, currentMax, torqueMin, torqueMax;
    
    if (direction == "Forward") {
        currentMin = m_recipe.loadForwardCurrentMin;
        currentMax = m_recipe.loadForwardCurrentMax;
        torqueMin = m_recipe.loadForwardTorqueMin;
        torqueMax = m_recipe.loadForwardTorqueMax;
    } else {
        currentMin = m_recipe.loadReverseCurrentMin;
        currentMax = m_recipe.loadReverseCurrentMax;
        torqueMin = m_recipe.loadReverseTorqueMin;
        torqueMax = m_recipe.loadReverseTorqueMax;
    }

    // Evaluate
    result.currentPassed = (lockCurrent >= currentMin) && (lockCurrent <= currentMax);
    result.torquePassed = (lockTorque >= torqueMin) && (lockTorque <= torqueMax);
    result.overallPassed = result.currentPassed && result.torquePassed;

    qDebug() << "Load" << direction << "results:"
             << "Current:" << lockCurrent << (result.currentPassed ? "PASS" : "FAIL")
             << "Torque:" << lockTorque << (result.torquePassed ? "PASS" : "FAIL");
}

bool GearboxTestEngine::checkLockCondition(const TelemetrySnapshot& snapshot) {
// Check speed threshold - if speed is too high, reset to idle
bool speedLow = qAbs(snapshot.dynSpeedRpm) <= m_recipe.lockSpeedThresholdRpm;

if (!speedLow) {
// Speed too high - reset to idle state
if (m_lockState != LockDetectionState::Idle) {
m_lockState = LockDetectionState::Idle;
m_lockTimer.invalidate();
qDebug() << "Lock detection reset: speed too high (" << snapshot.dynSpeedRpm << "RPM)";
}
return false;
}

// Speed is within threshold, proceed with state machine
switch (m_lockState) {
case LockDetectionState::Idle:
// Start window check phase
m_lockState = LockDetectionState::WindowCheck;
m_lockTimer.start();
m_lockReferenceAngle = snapshot.encoderAngleDeg;
return false;

case LockDetectionState::WindowCheck: {
// Check if we've completed the window check duration
if (m_lockTimer.elapsed() >= m_recipe.lockAngleWindowMs) {
// Check angle stability within window
double angleDelta = qAbs(snapshot.encoderAngleDeg - m_lockReferenceAngle);

if (angleDelta <= m_recipe.lockAngleDeltaDeg) {
// Angle stable, transition to hold check
m_lockState = LockDetectionState::HoldCheck;
m_lockTimer.restart();
qDebug() << "Lock window check passed, angle delta:" << angleDelta << "deg";
} else {
// Angle changed too much, restart window check with new reference
m_lockReferenceAngle = snapshot.encoderAngleDeg;
m_lockTimer.restart();
qDebug() << "Lock window check failed, angle delta:" << angleDelta << "deg, restarting";
}
}
return false;
}

case LockDetectionState::HoldCheck: {
// Check if we've held for required duration
if (m_lockTimer.elapsed() >= m_recipe.lockHoldMs) {
// Verify angle is still stable
double angleDelta = qAbs(snapshot.encoderAngleDeg - m_lockReferenceAngle);
if (angleDelta <= m_recipe.lockAngleDeltaDeg) {
m_lockState = LockDetectionState::Locked;
qDebug() << "Lock condition ACHIEVED - held for" << m_lockTimer.elapsed() << "ms";
return true;
} else {
// Angle drifted during hold, restart
m_lockState = LockDetectionState::WindowCheck;
m_lockTimer.restart();
m_lockReferenceAngle = snapshot.encoderAngleDeg;
qDebug() << "Lock hold failed, angle drifted:" << angleDelta << "deg";
}
}
return false;
}

case LockDetectionState::Locked:
// Already locked, maintain state
return true;
}

return false;
}

void GearboxTestEngine::handleSettlingForwardDelay() {
    m_state.statusMessage = "Settling (forward to reverse)...";
    if (m_state.phaseElapsedMs >= m_settlingTargetMs) {
        transitionToSubState(TestSubState::SpinupReverse);
    }
}

void GearboxTestEngine::handleSettlingReverseDelay() {
    m_state.statusMessage = "Settling (reverse to angle)...";
    if (m_state.phaseElapsedMs >= m_settlingTargetMs) {
        transitionToPhase(TestPhase::AnglePositioning, TestSubState::MoveToPosition1);
    }
}

void GearboxTestEngine::handleSettlingPosition2Delay() {
    m_state.statusMessage = "Settling after position 2...";
    if (m_state.phaseElapsedMs >= m_settlingTargetMs) {
        transitionToSubState(TestSubState::MoveBackToPosition1);
    }
}

void GearboxTestEngine::handleSettlingPosition1ReturnDelay() {
    m_state.statusMessage = "Settling after position 1 return...";
    if (m_state.phaseElapsedMs >= m_settlingTargetMs) {
        transitionToSubState(TestSubState::MoveToPosition3);
    }
}

void GearboxTestEngine::handleSettlingPosition3Delay() {
    m_state.statusMessage = "Settling after position 3...";
    if (m_state.phaseElapsedMs >= m_settlingTargetMs) {
        transitionToSubState(TestSubState::MoveBackToZero);
    }
}

void GearboxTestEngine::handleSettlingZeroDelay() {
    m_state.statusMessage = "Settling at zero before load test...";
    if (m_state.phaseElapsedMs >= m_settlingTargetMs) {
        transitionToPhase(TestPhase::LoadRampAndLock, TestSubState::SpinupLoadForward);
    }
}

void GearboxTestEngine::handleSettlingLoadForwardDelay() {
    m_state.statusMessage = "Settling after load forward...";
    if (m_state.phaseElapsedMs >= m_settlingTargetMs) {
        transitionToSubState(TestSubState::SpinupLoadReverse);
    }
}

void GearboxTestEngine::handleSettlingLoadReverseDelay() {
    m_state.statusMessage = "Settling after load reverse...";
    if (m_state.phaseElapsedMs >= m_settlingTargetMs) {
        transitionToPhase(TestPhase::ReturnToZero, TestSubState::ReturnFinalZero);
    }
}

} // namespace Domain
