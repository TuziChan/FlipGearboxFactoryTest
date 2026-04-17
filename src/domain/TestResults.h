#ifndef TESTRESULTS_H
#define TESTRESULTS_H

#include <QString>
#include <QDateTime>
#include <QVector>
#include "FailureReason.h"

namespace Domain {

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
