#include "RecipeConfig.h"
#include <QJsonValue>

namespace Infrastructure {
namespace Config {
namespace {

double readDouble(const QJsonObject& json, const char* key, double defaultValue) {
    const QJsonValue value = json.value(key);
    return value.isDouble() ? value.toDouble(defaultValue) : defaultValue;
}

int readInt(const QJsonObject& json, const char* key, int defaultValue) {
    const QJsonValue value = json.value(key);
    return value.isDouble() ? value.toInt(defaultValue) : defaultValue;
}

}

Domain::TestRecipe RecipeConfig::fromJson(const QJsonObject& json) {
    Domain::TestRecipe recipe;
    recipe.name = json["name"].toString(recipe.name);
    recipe.homeDutyCycle = readDouble(json, "homeDutyCycle", recipe.homeDutyCycle);
    recipe.homeAdvanceDutyCycle = readDouble(json, "homeAdvanceDutyCycle", recipe.homeAdvanceDutyCycle);
    recipe.encoderZeroAngleDeg = readDouble(json, "encoderZeroAngleDeg", recipe.encoderZeroAngleDeg);
    recipe.homeTimeoutMs = readInt(json, "homeTimeoutMs", recipe.homeTimeoutMs);
    recipe.idleDutyCycle = readDouble(json, "idleDutyCycle", recipe.idleDutyCycle);
    recipe.idleForwardSpinupMs = readInt(json, "idleForwardSpinupMs", recipe.idleForwardSpinupMs);
    recipe.idleForwardSampleMs = readInt(json, "idleForwardSampleMs", recipe.idleForwardSampleMs);
    recipe.idleReverseSpinupMs = readInt(json, "idleReverseSpinupMs", recipe.idleReverseSpinupMs);
    recipe.idleReverseSampleMs = readInt(json, "idleReverseSampleMs", recipe.idleReverseSampleMs);
    recipe.idleTimeoutMs = readInt(json, "idleTimeoutMs", recipe.idleTimeoutMs);
    recipe.idleForwardCurrentAvgMin = readDouble(json, "idleForwardCurrentAvgMin", recipe.idleForwardCurrentAvgMin);
    recipe.idleForwardCurrentAvgMax = readDouble(json, "idleForwardCurrentAvgMax", recipe.idleForwardCurrentAvgMax);
    recipe.idleForwardCurrentMaxMin = readDouble(json, "idleForwardCurrentMaxMin", recipe.idleForwardCurrentMaxMin);
    recipe.idleForwardCurrentMaxMax = readDouble(json, "idleForwardCurrentMaxMax", recipe.idleForwardCurrentMaxMax);
    recipe.idleForwardSpeedAvgMin = readDouble(json, "idleForwardSpeedAvgMin", recipe.idleForwardSpeedAvgMin);
    recipe.idleForwardSpeedAvgMax = readDouble(json, "idleForwardSpeedAvgMax", recipe.idleForwardSpeedAvgMax);
    recipe.idleForwardSpeedMaxMin = readDouble(json, "idleForwardSpeedMaxMin", recipe.idleForwardSpeedMaxMin);
    recipe.idleForwardSpeedMaxMax = readDouble(json, "idleForwardSpeedMaxMax", recipe.idleForwardSpeedMaxMax);
    recipe.idleReverseCurrentAvgMin = readDouble(json, "idleReverseCurrentAvgMin", recipe.idleReverseCurrentAvgMin);
    recipe.idleReverseCurrentAvgMax = readDouble(json, "idleReverseCurrentAvgMax", recipe.idleReverseCurrentAvgMax);
    recipe.idleReverseCurrentMaxMin = readDouble(json, "idleReverseCurrentMaxMin", recipe.idleReverseCurrentMaxMin);
    recipe.idleReverseCurrentMaxMax = readDouble(json, "idleReverseCurrentMaxMax", recipe.idleReverseCurrentMaxMax);
    recipe.idleReverseSpeedAvgMin = readDouble(json, "idleReverseSpeedAvgMin", recipe.idleReverseSpeedAvgMin);
    recipe.idleReverseSpeedAvgMax = readDouble(json, "idleReverseSpeedAvgMax", recipe.idleReverseSpeedAvgMax);
    recipe.idleReverseSpeedMaxMin = readDouble(json, "idleReverseSpeedMaxMin", recipe.idleReverseSpeedMaxMin);
    recipe.idleReverseSpeedMaxMax = readDouble(json, "idleReverseSpeedMaxMax", recipe.idleReverseSpeedMaxMax);
    recipe.angleTestDutyCycle = readDouble(json, "angleTestDutyCycle", recipe.angleTestDutyCycle);
    recipe.position1TargetDeg = readDouble(json, "position1TargetDeg", recipe.position1TargetDeg);
    recipe.position1ToleranceDeg = readDouble(json, "position1ToleranceDeg", recipe.position1ToleranceDeg);
    recipe.position2TargetDeg = readDouble(json, "position2TargetDeg", recipe.position2TargetDeg);
    recipe.position2ToleranceDeg = readDouble(json, "position2ToleranceDeg", recipe.position2ToleranceDeg);
    recipe.position3TargetDeg = readDouble(json, "position3TargetDeg", recipe.position3TargetDeg);
    recipe.position3ToleranceDeg = readDouble(json, "position3ToleranceDeg", recipe.position3ToleranceDeg);
    recipe.returnZeroToleranceDeg = readDouble(json, "returnZeroToleranceDeg", recipe.returnZeroToleranceDeg);
    recipe.angleTimeoutMs = readInt(json, "angleTimeoutMs", readInt(json, "angleMoveTimeoutMs", recipe.angleTimeoutMs));
    recipe.loadDutyCycle = readDouble(json, "loadDutyCycle", recipe.loadDutyCycle);
    recipe.loadTimeoutMs = readInt(json, "loadTimeoutMs", recipe.loadTimeoutMs);
    recipe.loadSpinupMs = readInt(json, "loadSpinupMs", recipe.loadSpinupMs);
    recipe.loadRampMs = readInt(json, "loadRampMs", recipe.loadRampMs);
    recipe.brakeRampStartCurrentA = readDouble(json, "brakeRampStartCurrentA", recipe.brakeRampStartCurrentA);
    recipe.brakeRampEndCurrentA = readDouble(json, "brakeRampEndCurrentA", recipe.brakeRampEndCurrentA);
    recipe.brakeMode = json.value("brakeMode").isString() ? json.value("brakeMode").toString() : recipe.brakeMode;
    recipe.brakeRampStartVoltage = readDouble(json, "brakeRampStartVoltage", recipe.brakeRampStartVoltage);
    recipe.brakeRampEndVoltage = readDouble(json, "brakeRampEndVoltage", recipe.brakeRampEndVoltage);
    recipe.lockSpeedThresholdRpm = readDouble(json, "lockSpeedThresholdRpm", recipe.lockSpeedThresholdRpm);
    recipe.lockAngleWindowMs = readInt(json, "lockAngleWindowMs", recipe.lockAngleWindowMs);
    recipe.lockAngleDeltaDeg = readDouble(json, "lockAngleDeltaDeg", recipe.lockAngleDeltaDeg);
    recipe.lockHoldMs = readInt(json, "lockHoldMs", recipe.lockHoldMs);
    recipe.loadForwardCurrentMin = readDouble(json, "loadForwardCurrentMin", recipe.loadForwardCurrentMin);
    recipe.loadForwardCurrentMax = readDouble(json, "loadForwardCurrentMax", recipe.loadForwardCurrentMax);
    recipe.loadForwardTorqueMin = readDouble(json, "loadForwardTorqueMin", recipe.loadForwardTorqueMin);
    recipe.loadForwardTorqueMax = readDouble(json, "loadForwardTorqueMax", recipe.loadForwardTorqueMax);
    recipe.loadReverseCurrentMin = readDouble(json, "loadReverseCurrentMin", recipe.loadReverseCurrentMin);
    recipe.loadReverseCurrentMax = readDouble(json, "loadReverseCurrentMax", recipe.loadReverseCurrentMax);
    recipe.loadReverseTorqueMin = readDouble(json, "loadReverseTorqueMin", recipe.loadReverseTorqueMin);
    recipe.loadReverseTorqueMax = readDouble(json, "loadReverseTorqueMax", recipe.loadReverseTorqueMax);
    recipe.returnZeroTimeoutMs = readInt(json, "returnZeroTimeoutMs", recipe.returnZeroTimeoutMs);
    recipe.gearBacklashCompensationDeg = readDouble(json, "gearBacklashCompensationDeg", recipe.gearBacklashCompensationDeg);
    return recipe;
}

QJsonObject RecipeConfig::toJson(const Domain::TestRecipe& recipe) {
    QJsonObject json;
    json["name"] = recipe.name;
    json["homeDutyCycle"] = recipe.homeDutyCycle;
    json["homeAdvanceDutyCycle"] = recipe.homeAdvanceDutyCycle;
    json["encoderZeroAngleDeg"] = recipe.encoderZeroAngleDeg;
    json["homeTimeoutMs"] = recipe.homeTimeoutMs;
    json["idleDutyCycle"] = recipe.idleDutyCycle;
    json["idleForwardSpinupMs"] = recipe.idleForwardSpinupMs;
    json["idleForwardSampleMs"] = recipe.idleForwardSampleMs;
    json["idleReverseSpinupMs"] = recipe.idleReverseSpinupMs;
    json["idleReverseSampleMs"] = recipe.idleReverseSampleMs;
    json["idleTimeoutMs"] = recipe.idleTimeoutMs;
    json["idleForwardCurrentAvgMin"] = recipe.idleForwardCurrentAvgMin;
    json["idleForwardCurrentAvgMax"] = recipe.idleForwardCurrentAvgMax;
    json["idleForwardCurrentMaxMin"] = recipe.idleForwardCurrentMaxMin;
    json["idleForwardCurrentMaxMax"] = recipe.idleForwardCurrentMaxMax;
    json["idleForwardSpeedAvgMin"] = recipe.idleForwardSpeedAvgMin;
    json["idleForwardSpeedAvgMax"] = recipe.idleForwardSpeedAvgMax;
    json["idleForwardSpeedMaxMin"] = recipe.idleForwardSpeedMaxMin;
    json["idleForwardSpeedMaxMax"] = recipe.idleForwardSpeedMaxMax;
    json["idleReverseCurrentAvgMin"] = recipe.idleReverseCurrentAvgMin;
    json["idleReverseCurrentAvgMax"] = recipe.idleReverseCurrentAvgMax;
    json["idleReverseCurrentMaxMin"] = recipe.idleReverseCurrentMaxMin;
    json["idleReverseCurrentMaxMax"] = recipe.idleReverseCurrentMaxMax;
    json["idleReverseSpeedAvgMin"] = recipe.idleReverseSpeedAvgMin;
    json["idleReverseSpeedAvgMax"] = recipe.idleReverseSpeedAvgMax;
    json["idleReverseSpeedMaxMin"] = recipe.idleReverseSpeedMaxMin;
    json["idleReverseSpeedMaxMax"] = recipe.idleReverseSpeedMaxMax;
    json["angleTestDutyCycle"] = recipe.angleTestDutyCycle;
    json["position1TargetDeg"] = recipe.position1TargetDeg;
    json["position1ToleranceDeg"] = recipe.position1ToleranceDeg;
    json["position2TargetDeg"] = recipe.position2TargetDeg;
    json["position2ToleranceDeg"] = recipe.position2ToleranceDeg;
    json["position3TargetDeg"] = recipe.position3TargetDeg;
    json["position3ToleranceDeg"] = recipe.position3ToleranceDeg;
    json["returnZeroToleranceDeg"] = recipe.returnZeroToleranceDeg;
    json["angleTimeoutMs"] = recipe.angleTimeoutMs;
    json["angleMoveTimeoutMs"] = recipe.angleTimeoutMs;
    json["loadDutyCycle"] = recipe.loadDutyCycle;
    json["loadTimeoutMs"] = recipe.loadTimeoutMs;
    json["loadSpinupMs"] = recipe.loadSpinupMs;
    json["loadRampMs"] = recipe.loadRampMs;
    json["brakeRampStartCurrentA"] = recipe.brakeRampStartCurrentA;
    json["brakeRampEndCurrentA"] = recipe.brakeRampEndCurrentA;
    json["brakeMode"] = recipe.brakeMode;
    json["brakeRampStartVoltage"] = recipe.brakeRampStartVoltage;
    json["brakeRampEndVoltage"] = recipe.brakeRampEndVoltage;
    json["lockSpeedThresholdRpm"] = recipe.lockSpeedThresholdRpm;
    json["lockAngleWindowMs"] = recipe.lockAngleWindowMs;
    json["lockAngleDeltaDeg"] = recipe.lockAngleDeltaDeg;
    json["lockHoldMs"] = recipe.lockHoldMs;
    json["loadForwardCurrentMin"] = recipe.loadForwardCurrentMin;
    json["loadForwardCurrentMax"] = recipe.loadForwardCurrentMax;
    json["loadForwardTorqueMin"] = recipe.loadForwardTorqueMin;
    json["loadForwardTorqueMax"] = recipe.loadForwardTorqueMax;
    json["loadReverseCurrentMin"] = recipe.loadReverseCurrentMin;
    json["loadReverseCurrentMax"] = recipe.loadReverseCurrentMax;
    json["loadReverseTorqueMin"] = recipe.loadReverseTorqueMin;
    json["loadReverseTorqueMax"] = recipe.loadReverseTorqueMax;
    json["returnZeroTimeoutMs"] = recipe.returnZeroTimeoutMs;
    json["gearBacklashCompensationDeg"] = recipe.gearBacklashCompensationDeg;
    return json;
}

Domain::TestRecipe RecipeConfig::createDefault() {
    // Use the default constructor which already has sensible defaults
    Domain::TestRecipe recipe;
    recipe.name = "GBX-42A Default";
    return recipe;
}

} // namespace Config
} // namespace Infrastructure
