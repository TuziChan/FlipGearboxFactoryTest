#ifndef TESTRECIPE_H
#define TESTRECIPE_H

#include <QString>

namespace Domain {

/**
 * @brief Test recipe configuration
 * 
 * Contains all parameters and limits for gearbox testing.
 */
struct TestRecipe {
    QString name;
    
    // Homing parameters
    double homeDutyCycle;           // % for seeking magnet
    double homeAdvanceDutyCycle;    // % for advancing to encoder zero
    double encoderZeroAngleDeg;     // Target encoder zero position
    int homeTimeoutMs;              // Timeout for homing phase
    
    // Idle run parameters
    double idleDutyCycle;           // % for idle test
    int idleForwardSpinupMs;        // Forward spinup time before sampling
    int idleForwardSampleMs;        // Forward sampling window duration
    int idleReverseSpinupMs;        // Reverse spinup time before sampling
    int idleReverseSampleMs;        // Reverse sampling window duration
    
    // Idle limits - forward
    double idleForwardCurrentAvgMin;
    double idleForwardCurrentAvgMax;
    double idleForwardCurrentMaxMin;
    double idleForwardCurrentMaxMax;
    double idleForwardSpeedAvgMin;
    double idleForwardSpeedAvgMax;
    double idleForwardSpeedMaxMin;
    double idleForwardSpeedMaxMax;
    
    // Idle limits - reverse
    double idleReverseCurrentAvgMin;
    double idleReverseCurrentAvgMax;
    double idleReverseCurrentMaxMin;
    double idleReverseCurrentMaxMax;
    double idleReverseSpeedAvgMin;
    double idleReverseSpeedAvgMax;
    double idleReverseSpeedMaxMin;
    double idleReverseSpeedMaxMax;
    
    // Angle positioning parameters
    double angleTestDutyCycle;      // % for angle test
    double position1TargetDeg;      // Target angle for position 1
    double position1ToleranceDeg;   // Tolerance for position 1
    double position2TargetDeg;      // Target angle for position 2
    double position2ToleranceDeg;   // Tolerance for position 2
    double position3TargetDeg;      // Target angle for position 3
    double position3ToleranceDeg;   // Tolerance for position 3
    double returnZeroToleranceDeg;  // Tolerance for return-to-zero
    int angleTimeoutMs;             // Timeout for each angle move
    
    // Load test parameters
    double loadDutyCycle;           // % for load test
    int loadSpinupMs;               // Spinup time before brake ramp
    int loadRampMs;                 // Brake current ramp duration
    double brakeRampStartCurrentA;  // Starting brake current
    double brakeRampEndCurrentA;    // Ending brake current
    double lockSpeedThresholdRpm;   // Speed threshold for lock detection
    int lockAngleWindowMs;          // Window for angle change check
    double lockAngleDeltaDeg;       // Max angle change in window
    int lockHoldMs;                 // Duration to hold lock condition
    
    // Load limits - forward
    double loadForwardCurrentMin;
    double loadForwardCurrentMax;
    double loadForwardTorqueMin;
    double loadForwardTorqueMax;
    
    // Load limits - reverse
    double loadReverseCurrentMin;
    double loadReverseCurrentMax;
    double loadReverseTorqueMin;
    double loadReverseTorqueMax;
    
    TestRecipe()
        : name("Default")
        , homeDutyCycle(20.0)
        , homeAdvanceDutyCycle(20.0)
        , encoderZeroAngleDeg(3.0)
        , homeTimeoutMs(30000)
        , idleDutyCycle(50.0)
        , idleForwardSpinupMs(3000)
        , idleForwardSampleMs(2000)
        , idleReverseSpinupMs(3000)
        , idleReverseSampleMs(2000)
        , idleForwardCurrentAvgMin(0.5)
        , idleForwardCurrentAvgMax(2.0)
        , idleForwardCurrentMaxMin(0.6)
        , idleForwardCurrentMaxMax(2.5)
        , idleForwardSpeedAvgMin(50.0)
        , idleForwardSpeedAvgMax(150.0)
        , idleForwardSpeedMaxMin(60.0)
        , idleForwardSpeedMaxMax(160.0)
        , idleReverseCurrentAvgMin(0.5)
        , idleReverseCurrentAvgMax(2.0)
        , idleReverseCurrentMaxMin(0.6)
        , idleReverseCurrentMaxMax(2.5)
        , idleReverseSpeedAvgMin(50.0)
        , idleReverseSpeedAvgMax(150.0)
        , idleReverseSpeedMaxMin(60.0)
        , idleReverseSpeedMaxMax(160.0)
        , angleTestDutyCycle(30.0)
        , position1TargetDeg(3.0)
        , position1ToleranceDeg(3.0)
        , position2TargetDeg(49.0)
        , position2ToleranceDeg(3.0)
        , position3TargetDeg(113.5)
        , position3ToleranceDeg(3.0)
        , returnZeroToleranceDeg(1.0)
        , angleTimeoutMs(15000)
        , loadDutyCycle(50.0)
        , loadSpinupMs(3000)
        , loadRampMs(2000)
        , brakeRampStartCurrentA(0.0)
        , brakeRampEndCurrentA(3.0)
        , lockSpeedThresholdRpm(2.0)
        , lockAngleWindowMs(100)
        , lockAngleDeltaDeg(5.0)
        , lockHoldMs(500)
        , loadForwardCurrentMin(1.0)
        , loadForwardCurrentMax(3.0)
        , loadForwardTorqueMin(10.0)
        , loadForwardTorqueMax(50.0)
        , loadReverseCurrentMin(1.0)
        , loadReverseCurrentMax(3.0)
        , loadReverseTorqueMin(10.0)
        , loadReverseTorqueMax(50.0)
    {}
};

} // namespace Domain

#endif // TESTRECIPE_H
