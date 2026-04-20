#include <QtTest/QtTest>
#include "../src/domain/RecipeValidator.h"
#include "../src/domain/TestRecipe.h"

using namespace Domain;

class BoundaryProtectionTests : public QObject {
    Q_OBJECT

private slots:
    void testValidRecipe();
    void testInvalidDutyCycle();
    void testInvalidTimeout();
    void testInvalidCurrentRange();
    void testInvalidVoltageRange();
    void testLogicalConsistency();
    void testBrakeModeValidation();
    void testTimeoutConsistency();
};

void BoundaryProtectionTests::testValidRecipe() {
    TestRecipe recipe;
    QStringList errors;
    
    QVERIFY(RecipeValidator::validate(recipe, errors));
    QVERIFY(errors.isEmpty());
}

void BoundaryProtectionTests::testInvalidDutyCycle() {
    TestRecipe recipe;
    recipe.homeDutyCycle = 150.0; // Invalid: > 100
    
    QStringList errors;
    QVERIFY(!RecipeValidator::validate(recipe, errors));
    QVERIFY(!errors.isEmpty());
    QVERIFY(errors.join(" ").contains("homeDutyCycle"));
}

void BoundaryProtectionTests::testInvalidTimeout() {
    TestRecipe recipe;
    recipe.homeTimeoutMs = 50; // Invalid: < 100
    
    QStringList errors;
    QVERIFY(!RecipeValidator::validate(recipe, errors));
    QVERIFY(!errors.isEmpty());
    QVERIFY(errors.join(" ").contains("homeTimeoutMs"));
}

void BoundaryProtectionTests::testInvalidCurrentRange() {
    TestRecipe recipe;
    recipe.brakeRampEndCurrentA = 60.0; // Invalid: > 50A max
    
    QStringList errors;
    QVERIFY(!RecipeValidator::validate(recipe, errors));
    QVERIFY(!errors.isEmpty());
    QVERIFY(errors.join(" ").contains("brakeRampEndCurrentA"));
}

void BoundaryProtectionTests::testInvalidVoltageRange() {
    TestRecipe recipe;
    recipe.brakeRampEndVoltage = 150.0; // Invalid: > 100V max
    
    QStringList errors;
    QVERIFY(!RecipeValidator::validate(recipe, errors));
    QVERIFY(!errors.isEmpty());
    QVERIFY(errors.join(" ").contains("brakeRampEndVoltage"));
}

void BoundaryProtectionTests::testLogicalConsistency() {
    TestRecipe recipe;
    recipe.idleForwardCurrentAvgMin = 2.0;
    recipe.idleForwardCurrentAvgMax = 1.0; // Invalid: min > max
    
    QStringList errors;
    QVERIFY(!RecipeValidator::validate(recipe, errors));
    QVERIFY(!errors.isEmpty());
    QVERIFY(errors.join(" ").contains("idleForwardCurrentAvgMin"));
}

void BoundaryProtectionTests::testBrakeModeValidation() {
    TestRecipe recipe;
    recipe.brakeMode = "INVALID"; // Invalid mode
    
    QStringList errors;
    QVERIFY(!RecipeValidator::validate(recipe, errors));
    QVERIFY(!errors.isEmpty());
    QVERIFY(errors.join(" ").contains("brakeMode"));
}

void BoundaryProtectionTests::testTimeoutConsistency() {
    TestRecipe recipe;
    recipe.idleForwardSpinupMs = 10000;
    recipe.idleForwardSampleMs = 10000;
    recipe.idleReverseSpinupMs = 10000;
    recipe.idleReverseSampleMs = 10000;
    recipe.idleTimeoutMs = 5000; // Invalid: timeout < sum of durations
    
    QStringList errors;
    QVERIFY(!RecipeValidator::validate(recipe, errors));
    QVERIFY(!errors.isEmpty());
    QVERIFY(errors.join(" ").contains("idleTimeoutMs"));
}

QTEST_MAIN(BoundaryProtectionTests)
#include "BoundaryProtectionTests.moc"
