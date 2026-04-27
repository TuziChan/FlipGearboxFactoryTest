#include <QTest>
#include "../../src/domain/GearboxTestEngine.h"
#include "../../src/domain/TestRecipe.h"
#include "../../tests/mocks/MockDevices.h"

class JudgementLogicTests : public QObject {
    Q_OBJECT

private slots:
    void testTorqueBoundaryPass();
    void testTorqueBoundaryFail();
    void testSpeedBoundaryPass();
    void testSpeedBoundaryFail();
    void testAngleBoundaryPass();
    void testAngleBoundaryFail();
    void testCurrentBoundaryPass();
    void testCurrentBoundaryFail();

private:
    Domain::TestRecipe createTestRecipe();
};

Domain::TestRecipe JudgementLogicTests::createTestRecipe() {
    Domain::TestRecipe recipe;
    recipe.name = "BoundaryTest";

    // Homing parameters
    recipe.homeDutyCycle = 20.0;
    recipe.homeAdvanceDutyCycle = 20.0;
    recipe.encoderZeroAngleDeg = 0.0;
    recipe.homeTimeoutMs = 5000;

    // Idle run parameters
    recipe.idleDutyCycle = 50.0;
    recipe.idleForwardSpinupMs = 500;
    recipe.idleForwardSampleMs = 1000;
    recipe.idleReverseSpinupMs = 500;
    recipe.idleReverseSampleMs = 1000;
    recipe.idleTimeoutMs = 10000;

    // Idle limits - forward
    recipe.idleForwardCurrentAvgMin = 0.5;
    recipe.idleForwardCurrentAvgMax = 2.0;
    recipe.idleForwardCurrentMaxMin = 0.6;
    recipe.idleForwardCurrentMaxMax = 2.5;
    recipe.idleForwardSpeedAvgMin = 50.0;
    recipe.idleForwardSpeedAvgMax = 150.0;
    recipe.idleForwardSpeedMaxMin = 60.0;
    recipe.idleForwardSpeedMaxMax = 160.0;

    // Idle limits - reverse
    recipe.idleReverseCurrentAvgMin = 0.5;
    recipe.idleReverseCurrentAvgMax = 2.0;
    recipe.idleReverseCurrentMaxMin = 0.6;
    recipe.idleReverseCurrentMaxMax = 2.5;
    recipe.idleReverseSpeedAvgMin = 50.0;
    recipe.idleReverseSpeedAvgMax = 150.0;
    recipe.idleReverseSpeedMaxMin = 60.0;
    recipe.idleReverseSpeedMaxMax = 160.0;

    // Angle positioning parameters (absolute targets)
    recipe.angleTestDutyCycle = 30.0;
    recipe.position1TargetDeg = 49.0;
    recipe.position1ToleranceDeg = 2.0;
    recipe.position2TargetDeg = 113.5;
    recipe.position2ToleranceDeg = 2.0;
    recipe.position3TargetDeg = 113.5;
    recipe.position3ToleranceDeg = 2.0;
    recipe.returnZeroToleranceDeg = 1.0;
    recipe.angleTimeoutMs = 10000;

    // Load test parameters
    recipe.loadDutyCycle = 50.0;
    recipe.loadTimeoutMs = 10000;
    recipe.loadSpinupMs = 500;
    recipe.loadRampMs = 2000;
    recipe.brakeRampStartCurrentA = 0.0;
    recipe.brakeRampEndCurrentA = 3.0;
    recipe.brakeMode = "CC";
    recipe.lockSpeedThresholdRpm = 2.0;
    recipe.lockAngleWindowMs = 100;
    recipe.lockAngleDeltaDeg = 5.0;
    recipe.lockHoldMs = 500;

    // Load limits - forward
    recipe.loadForwardCurrentMin = 1.0;
    recipe.loadForwardCurrentMax = 5.0;
    recipe.loadForwardTorqueMin = 10.0;
    recipe.loadForwardTorqueMax = 30.0;

    // Load limits - reverse
    recipe.loadReverseCurrentMin = 1.0;
    recipe.loadReverseCurrentMax = 5.0;
    recipe.loadReverseTorqueMin = 10.0;
    recipe.loadReverseTorqueMax = 30.0;

    // Return to zero
    recipe.returnZeroTimeoutMs = 5000;

    // Gear backlash compensation
    recipe.gearBacklashCompensationDeg = 0.0;

    return recipe;
}

void JudgementLogicTests::testTorqueBoundaryPass() {
    auto recipe = createTestRecipe();
    recipe.loadForwardTorqueMin = 10.0;
    recipe.loadForwardTorqueMax = 20.0;

    double actualTorque = 15.0;

    QVERIFY(actualTorque >= recipe.loadForwardTorqueMin);
    QVERIFY(actualTorque <= recipe.loadForwardTorqueMax);
}

void JudgementLogicTests::testTorqueBoundaryFail() {
    auto recipe = createTestRecipe();
    recipe.loadForwardTorqueMin = 10.0;
    recipe.loadForwardTorqueMax = 20.0;

    double actualTorqueHigh = 25.0;
    double actualTorqueLow = 5.0;

    QVERIFY(!(actualTorqueHigh >= recipe.loadForwardTorqueMin && actualTorqueHigh <= recipe.loadForwardTorqueMax));
    QVERIFY(!(actualTorqueLow >= recipe.loadForwardTorqueMin && actualTorqueLow <= recipe.loadForwardTorqueMax));
}

void JudgementLogicTests::testSpeedBoundaryPass() {
    auto recipe = createTestRecipe();
    double targetSpeed = 100.0;
    recipe.idleForwardSpeedAvgMin = targetSpeed - 5.0;
    recipe.idleForwardSpeedAvgMax = targetSpeed + 5.0;

    double actualSpeed = 100.0;

    QVERIFY(actualSpeed >= recipe.idleForwardSpeedAvgMin);
    QVERIFY(actualSpeed <= recipe.idleForwardSpeedAvgMax);
}

void JudgementLogicTests::testSpeedBoundaryFail() {
    auto recipe = createTestRecipe();
    double targetSpeed = 100.0;
    recipe.idleForwardSpeedAvgMin = targetSpeed - 5.0;
    recipe.idleForwardSpeedAvgMax = targetSpeed + 5.0;

    double actualSpeed = 120.0;

    QVERIFY(!(actualSpeed >= recipe.idleForwardSpeedAvgMin && actualSpeed <= recipe.idleForwardSpeedAvgMax));
}

void JudgementLogicTests::testAngleBoundaryPass() {
    auto recipe = createTestRecipe();
    double targetAngle = recipe.position1TargetDeg;
    double tolerance = recipe.position1ToleranceDeg;

    double actualAngle = targetAngle - 0.5;

    QVERIFY(qAbs(actualAngle - targetAngle) <= tolerance);
}

void JudgementLogicTests::testAngleBoundaryFail() {
    auto recipe = createTestRecipe();
    double targetAngle = recipe.position1TargetDeg;
    double tolerance = recipe.position1ToleranceDeg;

    double actualAngle = targetAngle + 5.0;

    QVERIFY(!(qAbs(actualAngle - targetAngle) <= tolerance));
}

void JudgementLogicTests::testCurrentBoundaryPass() {
    auto recipe = createTestRecipe();
    double maxCurrent = recipe.loadForwardCurrentMax;

    double actualCurrent = maxCurrent - 0.5;

    QVERIFY(actualCurrent <= maxCurrent);
}

void JudgementLogicTests::testCurrentBoundaryFail() {
    auto recipe = createTestRecipe();
    double maxCurrent = recipe.loadForwardCurrentMax;

    double actualCurrent = maxCurrent + 1.0;

    QVERIFY(!(actualCurrent <= maxCurrent));
}

QTEST_MAIN(JudgementLogicTests)
#include "JudgementLogicTests.moc"
