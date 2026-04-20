#include "RecipeValidator.h"

namespace Domain {

bool RecipeValidator::validate(const TestRecipe& recipe, QStringList& errors) {
    errors.clear();
    bool valid = true;

    // Homing parameters
    valid &= validateRange(recipe.homeDutyCycle, MIN_DUTY_CYCLE, MAX_DUTY_CYCLE, 
                          "homeDutyCycle", errors);
    valid &= validateRange(recipe.homeAdvanceDutyCycle, MIN_DUTY_CYCLE, MAX_DUTY_CYCLE, 
                          "homeAdvanceDutyCycle", errors);
    valid &= validateRange(recipe.encoderZeroAngleDeg, MIN_ANGLE, MAX_ANGLE, 
                          "encoderZeroAngleDeg", errors);
    valid &= validateRange(recipe.homeTimeoutMs, MIN_TIMEOUT_MS, MAX_TIMEOUT_MS, 
                          "homeTimeoutMs", errors);

    // Idle run parameters
    valid &= validateRange(recipe.idleDutyCycle, MIN_DUTY_CYCLE, MAX_DUTY_CYCLE, 
                          "idleDutyCycle", errors);
    valid &= validateRange(recipe.idleForwardSpinupMs, MIN_DURATION_MS, MAX_DURATION_MS, 
                          "idleForwardSpinupMs", errors);
    valid &= validateRange(recipe.idleForwardSampleMs, MIN_DURATION_MS, MAX_DURATION_MS, 
                          "idleForwardSampleMs", errors);
    valid &= validateRange(recipe.idleReverseSpinupMs, MIN_DURATION_MS, MAX_DURATION_MS, 
                          "idleReverseSpinupMs", errors);
    valid &= validateRange(recipe.idleReverseSampleMs, MIN_DURATION_MS, MAX_DURATION_MS, 
                          "idleReverseSampleMs", errors);
    valid &= validateRange(recipe.idleTimeoutMs, MIN_TIMEOUT_MS, MAX_TIMEOUT_MS, 
                          "idleTimeoutMs", errors);

    // Idle limits - forward
    valid &= validateRange(recipe.idleForwardCurrentAvgMin, MIN_CURRENT, MAX_CURRENT, 
                          "idleForwardCurrentAvgMin", errors);
    valid &= validateRange(recipe.idleForwardCurrentAvgMax, MIN_CURRENT, MAX_CURRENT, 
                          "idleForwardCurrentAvgMax", errors);
    valid &= validateRange(recipe.idleForwardCurrentMaxMin, MIN_CURRENT, MAX_CURRENT, 
                          "idleForwardCurrentMaxMin", errors);
    valid &= validateRange(recipe.idleForwardCurrentMaxMax, MIN_CURRENT, MAX_CURRENT, 
                          "idleForwardCurrentMaxMax", errors);
    valid &= validateRange(recipe.idleForwardSpeedAvgMin, MIN_SPEED, MAX_SPEED, 
                          "idleForwardSpeedAvgMin", errors);
    valid &= validateRange(recipe.idleForwardSpeedAvgMax, MIN_SPEED, MAX_SPEED, 
                          "idleForwardSpeedAvgMax", errors);
    valid &= validateRange(recipe.idleForwardSpeedMaxMin, MIN_SPEED, MAX_SPEED, 
                          "idleForwardSpeedMaxMin", errors);
    valid &= validateRange(recipe.idleForwardSpeedMaxMax, MIN_SPEED, MAX_SPEED, 
                          "idleForwardSpeedMaxMax", errors);

    // Idle limits - reverse
    valid &= validateRange(recipe.idleReverseCurrentAvgMin, MIN_CURRENT, MAX_CURRENT, 
                          "idleReverseCurrentAvgMin", errors);
    valid &= validateRange(recipe.idleReverseCurrentAvgMax, MIN_CURRENT, MAX_CURRENT, 
                          "idleReverseCurrentAvgMax", errors);
    valid &= validateRange(recipe.idleReverseCurrentMaxMin, MIN_CURRENT, MAX_CURRENT, 
                          "idleReverseCurrentMaxMin", errors);
    valid &= validateRange(recipe.idleReverseCurrentMaxMax, MIN_CURRENT, MAX_CURRENT, 
                          "idleReverseCurrentMaxMax", errors);
    valid &= validateRange(recipe.idleReverseSpeedAvgMin, MIN_SPEED, MAX_SPEED, 
                          "idleReverseSpeedAvgMin", errors);
    valid &= validateRange(recipe.idleReverseSpeedAvgMax, MIN_SPEED, MAX_SPEED, 
                          "idleReverseSpeedAvgMax", errors);
    valid &= validateRange(recipe.idleReverseSpeedMaxMin, MIN_SPEED, MAX_SPEED, 
                          "idleReverseSpeedMaxMin", errors);
    valid &= validateRange(recipe.idleReverseSpeedMaxMax, MIN_SPEED, MAX_SPEED, 
                          "idleReverseSpeedMaxMax", errors);

    // Angle positioning parameters
    valid &= validateRange(recipe.angleTestDutyCycle, MIN_DUTY_CYCLE, MAX_DUTY_CYCLE, 
                          "angleTestDutyCycle", errors);
    valid &= validateRange(recipe.position1TargetDeg, MIN_ANGLE, MAX_ANGLE, 
                          "position1TargetDeg", errors);
    valid &= validateRange(recipe.position1ToleranceDeg, MIN_TOLERANCE, MAX_TOLERANCE, 
                          "position1ToleranceDeg", errors);
    valid &= validateRange(recipe.position2TargetDeg, MIN_ANGLE, MAX_ANGLE, 
                          "position2TargetDeg", errors);
    valid &= validateRange(recipe.position2ToleranceDeg, MIN_TOLERANCE, MAX_TOLERANCE, 
                          "position2ToleranceDeg", errors);
    valid &= validateRange(recipe.position3TargetDeg, MIN_ANGLE, MAX_ANGLE, 
                          "position3TargetDeg", errors);
    valid &= validateRange(recipe.position3ToleranceDeg, MIN_TOLERANCE, MAX_TOLERANCE, 
                          "position3ToleranceDeg", errors);
    valid &= validateRange(recipe.returnZeroToleranceDeg, MIN_TOLERANCE, MAX_TOLERANCE, 
                          "returnZeroToleranceDeg", errors);
    valid &= validateRange(recipe.angleTimeoutMs, MIN_TIMEOUT_MS, MAX_TIMEOUT_MS, 
                          "angleTimeoutMs", errors);

    // Load test parameters
    valid &= validateRange(recipe.loadDutyCycle, MIN_DUTY_CYCLE, MAX_DUTY_CYCLE, 
                          "loadDutyCycle", errors);
    valid &= validateRange(recipe.loadTimeoutMs, MIN_TIMEOUT_MS, MAX_TIMEOUT_MS, 
                          "loadTimeoutMs", errors);
    valid &= validateRange(recipe.loadSpinupMs, MIN_DURATION_MS, MAX_DURATION_MS, 
                          "loadSpinupMs", errors);
    valid &= validateRange(recipe.loadRampMs, MIN_DURATION_MS, MAX_DURATION_MS, 
                          "loadRampMs", errors);
    valid &= validateRange(recipe.brakeRampStartCurrentA, MIN_CURRENT, MAX_CURRENT, 
                          "brakeRampStartCurrentA", errors);
    valid &= validateRange(recipe.brakeRampEndCurrentA, MIN_CURRENT, MAX_CURRENT, 
                          "brakeRampEndCurrentA", errors);
    valid &= validateRange(recipe.brakeRampStartVoltage, MIN_VOLTAGE, MAX_VOLTAGE, 
                          "brakeRampStartVoltage", errors);
    valid &= validateRange(recipe.brakeRampEndVoltage, MIN_VOLTAGE, MAX_VOLTAGE, 
                          "brakeRampEndVoltage", errors);
    valid &= validateRange(recipe.lockSpeedThresholdRpm, MIN_SPEED, MAX_SPEED, 
                          "lockSpeedThresholdRpm", errors);
    valid &= validatePositive(recipe.lockAngleWindowMs, "lockAngleWindowMs", errors);
    valid &= validateRange(recipe.lockAngleDeltaDeg, MIN_TOLERANCE, MAX_TOLERANCE, 
                          "lockAngleDeltaDeg", errors);
    valid &= validatePositive(recipe.lockHoldMs, "lockHoldMs", errors);

    // Brake mode validation
    if (recipe.brakeMode != "CC" && recipe.brakeMode != "CV") {
        errors.append(QString("brakeMode must be 'CC' or 'CV', got '%1'").arg(recipe.brakeMode));
        valid = false;
    }

    // Load limits - forward
    valid &= validateRange(recipe.loadForwardCurrentMin, MIN_CURRENT, MAX_CURRENT, 
                          "loadForwardCurrentMin", errors);
    valid &= validateRange(recipe.loadForwardCurrentMax, MIN_CURRENT, MAX_CURRENT, 
                          "loadForwardCurrentMax", errors);
    valid &= validateRange(recipe.loadForwardTorqueMin, MIN_TORQUE, MAX_TORQUE, 
                          "loadForwardTorqueMin", errors);
    valid &= validateRange(recipe.loadForwardTorqueMax, MIN_TORQUE, MAX_TORQUE, 
                          "loadForwardTorqueMax", errors);

    // Load limits - reverse
    valid &= validateRange(recipe.loadReverseCurrentMin, MIN_CURRENT, MAX_CURRENT, 
                          "loadReverseCurrentMin", errors);
    valid &= validateRange(recipe.loadReverseCurrentMax, MIN_CURRENT, MAX_CURRENT, 
                          "loadReverseCurrentMax", errors);
    valid &= validateRange(recipe.loadReverseTorqueMin, MIN_TORQUE, MAX_TORQUE, 
                          "loadReverseTorqueMin", errors);
    valid &= validateRange(recipe.loadReverseTorqueMax, MIN_TORQUE, MAX_TORQUE, 
                          "loadReverseTorqueMax", errors);

    // Return to zero
    valid &= validateRange(recipe.returnZeroTimeoutMs, MIN_TIMEOUT_MS, MAX_TIMEOUT_MS, 
                          "returnZeroTimeoutMs", errors);

    // Gear backlash compensation
    valid &= validateRange(recipe.gearBacklashCompensationDeg, MIN_ANGLE, MAX_ANGLE, 
                          "gearBacklashCompensationDeg", errors);

    // Logical consistency checks
    if (recipe.idleForwardCurrentAvgMin > recipe.idleForwardCurrentAvgMax) {
        errors.append("idleForwardCurrentAvgMin must be <= idleForwardCurrentAvgMax");
        valid = false;
    }
    if (recipe.idleForwardCurrentMaxMin > recipe.idleForwardCurrentMaxMax) {
        errors.append("idleForwardCurrentMaxMin must be <= idleForwardCurrentMaxMax");
        valid = false;
    }
    if (recipe.idleForwardSpeedAvgMin > recipe.idleForwardSpeedAvgMax) {
        errors.append("idleForwardSpeedAvgMin must be <= idleForwardSpeedAvgMax");
        valid = false;
    }
    if (recipe.idleForwardSpeedMaxMin > recipe.idleForwardSpeedMaxMax) {
        errors.append("idleForwardSpeedMaxMin must be <= idleForwardSpeedMaxMax");
        valid = false;
    }

    if (recipe.idleReverseCurrentAvgMin > recipe.idleReverseCurrentAvgMax) {
        errors.append("idleReverseCurrentAvgMin must be <= idleReverseCurrentAvgMax");
        valid = false;
    }
    if (recipe.idleReverseCurrentMaxMin > recipe.idleReverseCurrentMaxMax) {
        errors.append("idleReverseCurrentMaxMin must be <= idleReverseCurrentMaxMax");
        valid = false;
    }
    if (recipe.idleReverseSpeedAvgMin > recipe.idleReverseSpeedAvgMax) {
        errors.append("idleReverseSpeedAvgMin must be <= idleReverseSpeedAvgMax");
        valid = false;
    }
    if (recipe.idleReverseSpeedMaxMin > recipe.idleReverseSpeedMaxMax) {
        errors.append("idleReverseSpeedMaxMin must be <= idleReverseSpeedMaxMax");
        valid = false;
    }

    if (recipe.loadForwardCurrentMin > recipe.loadForwardCurrentMax) {
        errors.append("loadForwardCurrentMin must be <= loadForwardCurrentMax");
        valid = false;
    }
    if (recipe.loadForwardTorqueMin > recipe.loadForwardTorqueMax) {
        errors.append("loadForwardTorqueMin must be <= loadForwardTorqueMax");
        valid = false;
    }

    if (recipe.loadReverseCurrentMin > recipe.loadReverseCurrentMax) {
        errors.append("loadReverseCurrentMin must be <= loadReverseCurrentMax");
        valid = false;
    }
    if (recipe.loadReverseTorqueMin > recipe.loadReverseTorqueMax) {
        errors.append("loadReverseTorqueMin must be <= loadReverseTorqueMax");
        valid = false;
    }

    if (recipe.brakeRampStartCurrentA > recipe.brakeRampEndCurrentA) {
        errors.append("brakeRampStartCurrentA must be <= brakeRampEndCurrentA");
        valid = false;
    }

    if (recipe.brakeRampStartVoltage > recipe.brakeRampEndVoltage) {
        errors.append("brakeRampStartVoltage must be <= brakeRampEndVoltage");
        valid = false;
    }

    // Timeout consistency
    int totalIdleTime = recipe.idleForwardSpinupMs + recipe.idleForwardSampleMs + 
                        recipe.idleReverseSpinupMs + recipe.idleReverseSampleMs;
    if (totalIdleTime > recipe.idleTimeoutMs) {
        errors.append(QString("idleTimeoutMs (%1) must be >= sum of idle durations (%2)")
                     .arg(recipe.idleTimeoutMs).arg(totalIdleTime));
        valid = false;
    }

    int totalLoadTime = recipe.loadSpinupMs + recipe.loadRampMs + recipe.lockHoldMs;
    if (totalLoadTime * 2 > recipe.loadTimeoutMs) { // *2 for forward and reverse
        errors.append(QString("loadTimeoutMs (%1) should be >= 2x sum of load durations (%2)")
                     .arg(recipe.loadTimeoutMs).arg(totalLoadTime * 2));
        valid = false;
    }

    return valid;
}

bool RecipeValidator::validateRange(double value, double min, double max, 
                                    const QString& paramName, QStringList& errors) {
    if (value < min || value > max) {
        errors.append(QString("%1 = %2 is out of range [%3, %4]")
                     .arg(paramName).arg(value).arg(min).arg(max));
        return false;
    }
    return true;
}

bool RecipeValidator::validateRange(int value, int min, int max, 
                                    const QString& paramName, QStringList& errors) {
    if (value < min || value > max) {
        errors.append(QString("%1 = %2 is out of range [%3, %4]")
                     .arg(paramName).arg(value).arg(min).arg(max));
        return false;
    }
    return true;
}

bool RecipeValidator::validatePositive(double value, const QString& paramName, QStringList& errors) {
    if (value <= 0.0) {
        errors.append(QString("%1 = %2 must be positive").arg(paramName).arg(value));
        return false;
    }
    return true;
}

bool RecipeValidator::validatePositive(int value, const QString& paramName, QStringList& errors) {
    if (value <= 0) {
        errors.append(QString("%1 = %2 must be positive").arg(paramName).arg(value));
        return false;
    }
    return true;
}

} // namespace Domain
