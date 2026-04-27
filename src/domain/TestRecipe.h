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
    // Encoder zero is fixed at installation (~0°), not set dynamically during homing.
    // All position targets are absolute angles:
    //   Position 1: 49.0°
    //   Position 2: 113.5°
    //   Position 3: 113.5° (backlash check)
    double angleTestDutyCycle;      // % for angle test
    double position1TargetDeg;      // Absolute target angle for position 1
    double position1ToleranceDeg;   // Tolerance for position 1
    double position2TargetDeg;      // Absolute target angle for position 2
    double position2ToleranceDeg;   // Tolerance for position 2
    double position3TargetDeg;      // Absolute target angle for position 3
    double position3ToleranceDeg;   // Tolerance for position 3
    double returnZeroToleranceDeg;  // Tolerance for return-to-zero
    int angleTimeoutMs;             // Timeout for each angle move
    
    // Idle run timeout
    int idleTimeoutMs;              // Timeout for entire idle run phase

    // Load test parameters
    double loadDutyCycle;           // % for load test
    int loadTimeoutMs;              // Timeout for entire load test phase
    int loadSpinupMs;               // Spinup time before brake ramp
    int loadRampMs;                 // Brake current ramp duration
    double brakeRampStartCurrentA;  // Starting brake current
    double brakeRampEndCurrentA;    // Ending brake current
    QString brakeMode;               // "CC" or "CV", default "CC"
    double brakeRampStartVoltage;    // Starting voltage for CV mode ramp
    double brakeRampEndVoltage;      // Ending voltage for CV mode ramp
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

    // Return to zero
    int returnZeroTimeoutMs;        // Timeout for return-to-zero phase

    // Settling delays (non-blocking pauses between sub-states)
    int settlingAngleMoveMs;        // Settling between angle position moves (e.g., position 2→1)
    int settlingPhaseChangeMs;      // Settling between major phases (e.g., idle→angle, zero→load)

    // Gear backlash compensation
    double gearBacklashCompensationDeg; // Added to target angle during execution

    // Serial number generation rule
    QString serialNumberRule;           // e.g. yyyymmdd00001

    // Impact test parameters
    bool impactTestEnabled;            // Enable impact test phase (default false)
    double impactDutyCycle;            // % for impact spinup (default 50.0)
    int impactSpinupMs;                // Spinup time before impact braking (default 3000)
    int impactCycles;                  // Number of brake/release cycles per direction (default 3)
    double impactBrakeCurrentA;        // Brake current for impact (default 5.0)
    int impactBrakeOnMs;               // Duration to hold brake on (default 2000)
    int impactBrakeOffMs;              // Duration between brake cycles (default 500)
    int impactTimeoutMs;               // Total timeout for impact phase (default 60000)

    // Impact test current/torque limits
    double impactForwardCurrentMin;
    double impactForwardCurrentMax;
    double impactForwardTorqueMin;
    double impactForwardTorqueMax;
    double impactReverseCurrentMin;
    double impactReverseCurrentMax;
    double impactReverseTorqueMin;
    double impactReverseTorqueMax;

    TestRecipe()
        : name("Default")
        , homeDutyCycle(20.0)
        , homeAdvanceDutyCycle(20.0)
        , encoderZeroAngleDeg(0.0)
        , homeTimeoutMs(30000)
        , idleDutyCycle(50.0)
        , idleForwardSpinupMs(3000)
        , idleForwardSampleMs(2000)
        , idleReverseSpinupMs(3000)
        , idleReverseSampleMs(2000)
        , idleTimeoutMs(60000)
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
        , position1TargetDeg(49.0)     // Absolute: second magnet (zero point is fixed at ~0°, magnet at 49°)
        , position1ToleranceDeg(3.0)
        , position2TargetDeg(113.5)    // Absolute: third magnet (113.5°)
        , position2ToleranceDeg(3.0)
        , position3TargetDeg(113.5)    // Absolute: same as position 2 (backlash check)
        , position3ToleranceDeg(3.0)
        , returnZeroToleranceDeg(1.0)
        , angleTimeoutMs(15000)
        , loadDutyCycle(50.0)
        , loadTimeoutMs(60000)
        , loadSpinupMs(3000)
        , loadRampMs(2000)
        , brakeRampStartCurrentA(0.0)
        , brakeRampEndCurrentA(3.0)
        , brakeMode("CC")
        , brakeRampStartVoltage(0.0)
        , brakeRampEndVoltage(12.0)
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
        , returnZeroTimeoutMs(15000)
        , settlingAngleMoveMs(200)
        , settlingPhaseChangeMs(500)
        , gearBacklashCompensationDeg(0.0)
        , serialNumberRule("yyyymmdd00001")
        , impactTestEnabled(false)
        , impactDutyCycle(50.0)
        , impactSpinupMs(3000)
        , impactCycles(3)
        , impactBrakeCurrentA(5.0)
        , impactBrakeOnMs(2000)
        , impactBrakeOffMs(500)
        , impactTimeoutMs(60000)
        , impactForwardCurrentMin(1.0)
        , impactForwardCurrentMax(5.0)
        , impactForwardTorqueMin(5.0)
        , impactForwardTorqueMax(50.0)
        , impactReverseCurrentMin(1.0)
        , impactReverseCurrentMax(5.0)
        , impactReverseTorqueMin(5.0)
        , impactReverseTorqueMax(50.0)
    {}
};

} // namespace Domain

#endif // TESTRECIPE_H
