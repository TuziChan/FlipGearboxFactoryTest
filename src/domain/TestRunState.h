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
    HoldAfterPosition1,
    MoveToPosition2,
    HoldAfterPosition2,
    MoveBackToPosition1,
    HoldAfterPosition1Return,
    MoveToPosition3,
    HoldAfterPosition3,
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
