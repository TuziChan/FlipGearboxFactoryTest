#include "PhysicsValidator.h"
#include <cmath>
#include <algorithm>

namespace Infrastructure {
namespace Validation {

PhysicsValidator::ValidationResult PhysicsValidator::checkSpeedConsistency(
    const Domain::TelemetrySnapshot& snapshot,
    const ValidationConfig& config)
{
    ValidationResult result;
    result.ruleName = "R1_SpeedConsistency";

    // Skip if devices offline
    if (!snapshot.torqueOnline || !snapshot.encoderOnline) {
        result.passed = true;
        result.message = "Skipped (devices offline)";
        return result;
    }

    double speedTorque = std::abs(snapshot.dynSpeedRpm);
    double speedEncoder = std::abs(snapshot.encoderVelocityRpm);

    // Skip if both speeds near zero (no meaningful comparison)
    if (speedTorque < 10.0 && speedEncoder < 10.0) {
        result.passed = true;
        result.message = "Skipped (speeds near zero)";
        return result;
    }

    double maxSpeed = std::max(speedTorque, speedEncoder);
    double speedDiff = std::abs(speedTorque - speedEncoder);
    double errorPercent = (maxSpeed > 0.0) ? (speedDiff / maxSpeed) : 0.0;

    result.actualValue = speedTorque;
    result.expectedValue = speedEncoder;
    result.threshold = config.speedConsistencyThreshold;
    result.errorPercent = errorPercent;
    result.passed = (errorPercent <= config.speedConsistencyThreshold);

    if (!result.passed) {
        result.message = QString("Speed mismatch: Torque=%1 RPM, Encoder=%2 RPM, Error=%3%")
            .arg(speedTorque, 0, 'f', 1)
            .arg(speedEncoder, 0, 'f', 1)
            .arg(errorPercent * 100.0, 0, 'f', 2);
    }

    return result;
}

PhysicsValidator::ValidationResult PhysicsValidator::checkAccelerationLimit(
    const Domain::TelemetrySnapshot& current,
    const Domain::TelemetrySnapshot& previous,
    const ValidationConfig& config)
{
    ValidationResult result;
    result.ruleName = "R2_AccelerationLimit";

    // Skip if devices offline
    if (!current.encoderOnline || !previous.encoderOnline) {
        result.passed = true;
        result.message = "Skipped (devices offline)";
        return result;
    }

    // Calculate time delta in seconds
    qint64 deltaMs = previous.timestamp.msecsTo(current.timestamp);
    if (deltaMs <= 0) {
        result.passed = true;
        result.message = "Skipped (invalid time delta)";
        return result;
    }

    double deltaS = deltaMs / 1000.0;

    // Calculate acceleration (using encoder speed as reference)
    double speedDiff = current.encoderVelocityRpm - previous.encoderVelocityRpm;
    double accelerationRpmPerS = std::abs(speedDiff / deltaS);

    result.actualValue = accelerationRpmPerS;
    result.expectedValue = config.maxAccelerationRpmPerS;
    result.threshold = config.maxAccelerationRpmPerS;
    result.errorPercent = (accelerationRpmPerS > config.maxAccelerationRpmPerS)
        ? ((accelerationRpmPerS - config.maxAccelerationRpmPerS) / config.maxAccelerationRpmPerS)
        : 0.0;
    result.passed = (accelerationRpmPerS <= config.maxAccelerationRpmPerS);

    if (!result.passed) {
        result.message = QString("Acceleration too high: %1 RPM/s (limit: %2 RPM/s)")
            .arg(accelerationRpmPerS, 0, 'f', 1)
            .arg(config.maxAccelerationRpmPerS, 0, 'f', 1);
    }

    return result;
}

PhysicsValidator::ValidationResult PhysicsValidator::checkPowerConservation(
    const Domain::TelemetrySnapshot& snapshot,
    const ValidationConfig& config)
{
    ValidationResult result;
    result.ruleName = "R4_PowerConservation";

    // Skip if torque sensor offline
    if (!snapshot.torqueOnline) {
        result.passed = true;
        result.message = "Skipped (torque sensor offline)";
        return result;
    }

    double measuredPower = snapshot.dynPowerW;

    // Skip if power near zero (no meaningful comparison)
    if (std::abs(measuredPower) < 1.0) {
        result.passed = true;
        result.message = "Skipped (power near zero)";
        return result;
    }

    // Calculate power from torque and speed: P = T × ω
    // ω (rad/s) = RPM × 2π/60
    double torqueNm = snapshot.dynTorqueNm;
    double speedRpm = snapshot.dynSpeedRpm;
    double omegaRadPerS = speedRpm * 2.0 * M_PI / 60.0;
    double calculatedPower = torqueNm * omegaRadPerS;

    double powerDiff = std::abs(measuredPower - calculatedPower);
    double errorPercent = (std::abs(measuredPower) > 0.0)
        ? (powerDiff / std::abs(measuredPower))
        : 0.0;

    result.actualValue = measuredPower;
    result.expectedValue = calculatedPower;
    result.threshold = config.powerConservationThreshold;
    result.errorPercent = errorPercent;
    result.passed = (errorPercent <= config.powerConservationThreshold);

    if (!result.passed) {
        result.message = QString("Power mismatch: Measured=%1 W, Calculated=%2 W, Error=%3%")
            .arg(measuredPower, 0, 'f', 2)
            .arg(calculatedPower, 0, 'f', 2)
            .arg(errorPercent * 100.0, 0, 'f', 2);
    }

    return result;
}

PhysicsValidator::ValidationResult PhysicsValidator::checkBrakePower(
    const Domain::TelemetrySnapshot& snapshot,
    const ValidationConfig& config)
{
    ValidationResult result;
    result.ruleName = "R7_BrakePower";

    // Skip if brake offline
    if (!snapshot.brakeOnline) {
        result.passed = true;
        result.message = "Skipped (brake offline)";
        return result;
    }

    double measuredPower = snapshot.brakePowerW;

    // Skip if power near zero
    if (std::abs(measuredPower) < 0.1) {
        result.passed = true;
        result.message = "Skipped (brake power near zero)";
        return result;
    }

    // Calculate electrical power: P = V × I
    double calculatedPower = snapshot.brakeVoltageV * snapshot.brakeCurrentA;

    double powerDiff = std::abs(measuredPower - calculatedPower);
    double errorPercent = (std::abs(measuredPower) > 0.0)
        ? (powerDiff / std::abs(measuredPower))
        : 0.0;

    result.actualValue = measuredPower;
    result.expectedValue = calculatedPower;
    result.threshold = config.brakePowerThreshold;
    result.errorPercent = errorPercent;
    result.passed = (errorPercent <= config.brakePowerThreshold);

    if (!result.passed) {
        result.message = QString("Brake power mismatch: Measured=%1 W, Calculated=%2 W, Error=%3%")
            .arg(measuredPower, 0, 'f', 2)
            .arg(calculatedPower, 0, 'f', 2)
            .arg(errorPercent * 100.0, 0, 'f', 2);
    }

    return result;
}

QVector<PhysicsValidator::ValidationResult> PhysicsValidator::validateAll(
    const Domain::TelemetrySnapshot& current,
    const Domain::TelemetrySnapshot& previous,
    const ValidationConfig& config)
{
    QVector<ValidationResult> results;

    results.append(checkSpeedConsistency(current, config));
    results.append(checkAccelerationLimit(current, previous, config));
    results.append(checkPowerConservation(current, config));
    results.append(checkBrakePower(current, config));

    return results;
}

} // namespace Validation
} // namespace Infrastructure
