#ifndef TESTRUNSTATE_H
#define TESTRUNSTATE_H

#include <QString>
#include "TelemetrySnapshot.h"
#include "TestResults.h"

namespace Domain {

/**
 * @brief Main test phases
 */
enum class TestPhase {
    Idle,
    ImpactTest,
    PrepareAndHome,
    IdleRun,
    AnglePositioning,
    LoadRampAndLock,
    ReturnToZero,
    Completed,
    Failed
};

/**
 * @brief Detailed sub-states for each phase
 */
enum class TestSubState {
    // Idle
    NotStarted,

    // Impact test
    ImpactForwardSpinup,
    ImpactForwardBrakeOn,
    ImpactForwardBrakeOff,
    ImpactReverseSpinup,
    ImpactReverseBrakeOn,
    ImpactReverseBrakeOff,

    // Homing
    SeekingMagnet,
    AdvancingToEncoderZero,
    HomeSettled,
    
    // Idle run
    SpinupForward,
    SampleForward,
    SpinupReverse,
    SampleReverse,
    
    // Angle positioning
    MoveToPosition1,
    MoveToPosition2,
    MoveBackToPosition1,
    MoveToPosition3,
    MoveBackToZero,
    
    // Load test
    SpinupLoadForward,
    RampBrakeForward,
    ConfirmLockForward,
    SpinupLoadReverse,
    RampBrakeReverse,
    ConfirmLockReverse,
    
    // Return to zero
    ReturnFinalZero,
    FinalZeroSettled,
    
    // Settling delays (non-blocking)
    SettlingForwardDelay,
    SettlingReverseDelay,
    SettlingPosition2Delay,
    SettlingPosition1ReturnDelay,
    SettlingPosition3Delay,
    SettlingZeroDelay,
    SettlingLoadForwardDelay,
    SettlingLoadReverseDelay,

    // Terminal
    TestCompleted,
    TestFailed
};

/**
 * @brief Current motor direction
 */
enum class MotorDirection {
    Stopped,
    Forward,
    Reverse
};

/**
 * @brief Runtime state of the test engine
 * 
 * Projected to ViewModels for UI display.
 */
struct TestRunState {
    TestPhase phase;
    TestSubState subState;
    MotorDirection currentDirection;
    
    qint64 elapsedMs;
    qint64 phaseElapsedMs;
    
    // Current telemetry
    TelemetrySnapshot currentTelemetry;
    
    // Intermediate and final results
    TestResults results;
    
    // Progress indicators
    QString statusMessage;
    int progressPercent;  // 0-100
    
    TestRunState()
        : phase(TestPhase::Idle)
        , subState(TestSubState::NotStarted)
        , currentDirection(MotorDirection::Stopped)
        , elapsedMs(0)
        , phaseElapsedMs(0)
        , currentTelemetry()
        , results()
        , statusMessage("Ready")
        , progressPercent(0)
    {}
    
    QString phaseString() const {
        switch (phase) {
            case TestPhase::Idle: return "Idle";
            case TestPhase::ImpactTest: return "Impact Test";
            case TestPhase::PrepareAndHome: return "Homing";
            case TestPhase::IdleRun: return "Idle Run Test";
            case TestPhase::AnglePositioning: return "Angle Positioning";
            case TestPhase::LoadRampAndLock: return "Load Test";
            case TestPhase::ReturnToZero: return "Returning to Zero";
            case TestPhase::Completed: return "Completed";
            case TestPhase::Failed: return "Failed";
            default: return "Unknown";
        }
    }
};

} // namespace Domain

#endif // TESTRUNSTATE_H
