#ifndef TESTRESULTS_H
#define TESTRESULTS_H

#include <QString>
#include <QDateTime>
#include <QVector>
#include "FailureReason.h"

namespace Domain {

/**
 * @brief Individual impact cycle result
 */
struct ImpactCycleResult {
    int cycleNumber;       // 1-based cycle index
    double peakCurrentA;   // Peak current during this brake cycle
    double peakTorqueNm;   // Peak torque during this brake cycle
    double avgCurrentA;    // Average current while brake is on
    double avgTorqueNm;    // Average torque while brake is on
};

/**
 * @brief Impact test result for one direction
 */
struct ImpactDirectionResult {
    QString direction;     // "Forward" or "Reverse"
    QVector<ImpactCycleResult> cycles;
    double maxCurrentA;    // Max current across all cycles in this direction
    double maxTorqueNm;    // Max torque across all cycles
    double avgCurrentA;    // Average current across all cycles
    double avgTorqueNm;    // Average torque across all cycles
    bool currentPassed;
    bool torquePassed;
    bool overallPassed;

    ImpactDirectionResult()
        : direction(), cycles()
        , maxCurrentA(0.0), maxTorqueNm(0.0)
        , avgCurrentA(0.0), avgTorqueNm(0.0)
        , currentPassed(false), torquePassed(false), overallPassed(false)
    {}
};

/**
 * @brief Individual angle measurement result
 */
struct AngleResult {
    QString positionName;       // "Position 1", "Position 2", etc.
    double targetAngleDeg;
    double measuredAngleDeg;
    double deviationDeg;
    double toleranceDeg;
    bool passed;
    
    AngleResult()
        : positionName()
        , targetAngleDeg(0.0)
        , measuredAngleDeg(0.0)
        , deviationDeg(0.0)
        , toleranceDeg(0.0)
        , passed(false)
    {}
};

/**
 * @brief Idle run test result for one direction
 */
struct IdleRunResult {
    QString direction;          // "Forward" or "Reverse"
    double currentAvg;
    double currentMax;
    double speedAvg;
    double speedMax;
    bool currentAvgPassed;
    bool currentMaxPassed;
    bool speedAvgPassed;
    bool speedMaxPassed;
    bool overallPassed;
    
    IdleRunResult()
        : direction()
        , currentAvg(0.0)
        , currentMax(0.0)
        , speedAvg(0.0)
        , speedMax(0.0)
        , currentAvgPassed(false)
        , currentMaxPassed(false)
        , speedAvgPassed(false)
        , speedMaxPassed(false)
        , overallPassed(false)
    {}
};

/**
 * @brief Load test result for one direction
 */
struct LoadTestResult {
    QString direction;          // "Forward" or "Reverse"
    double lockCurrentA;
    double lockTorqueNm;
    bool currentPassed;
    bool torquePassed;
    bool overallPassed;
    bool lockAchieved;          // Whether lock condition was met
    
    LoadTestResult()
        : direction()
        , lockCurrentA(0.0)
        , lockTorqueNm(0.0)
        , currentPassed(false)
        , torquePassed(false)
        , overallPassed(false)
        , lockAchieved(false)
    {}
};

/**
 * @brief Complete test results
 */
struct TestResults {
    QDateTime startTime;
    QDateTime endTime;
    QString serialNumber;
    QString recipeName;
    QString stationName;
    
    // Homing
    bool homingCompleted;
    double finalEncoderZeroDeg;

    // Impact test
    ImpactDirectionResult impactForward;
    ImpactDirectionResult impactReverse;
    bool impactTestCompleted;  // true even when impact test is skipped (if disabled)

    // Idle run
    IdleRunResult idleForward;
    IdleRunResult idleReverse;
    
    // Angle positioning
    QVector<AngleResult> angleResults;
    
    // Load test
    LoadTestResult loadForward;
    LoadTestResult loadReverse;
    
    // Overall verdict
    bool overallPassed;
    FailureReason failure;
    
    TestResults()
        : startTime()
        , endTime()
        , serialNumber()
        , recipeName()
        , stationName()
        , homingCompleted(false)
        , finalEncoderZeroDeg(0.0)
        , impactForward()
        , impactReverse()
        , impactTestCompleted(true)
        , idleForward()
        , idleReverse()
        , angleResults()
        , loadForward()
        , loadReverse()
        , overallPassed(false)
        , failure()
    {}
};

} // namespace Domain

#endif // TESTRESULTS_H
