#ifndef PHYSICSVALIDATOR_H
#define PHYSICSVALIDATOR_H

#include <QString>
#include <QVector>
#include "../../domain/TelemetrySnapshot.h"

namespace Infrastructure {
namespace Validation {

/**
 * @brief Runtime physics validation for mock simulation
 *
 * Validates physical laws during test execution:
 * - R1: Speed consistency (torque sensor vs encoder)
 * - R2: Acceleration limits
 * - R4: Power conservation
 * - R7: Brake power dissipation
 */
class PhysicsValidator {
public:
    struct ValidationResult {
        bool passed;
        QString ruleName;
        QString message;
        double actualValue;
        double expectedValue;
        double threshold;
        double errorPercent;

        ValidationResult()
            : passed(true)
            , actualValue(0.0)
            , expectedValue(0.0)
            , threshold(0.0)
            , errorPercent(0.0)
        {}
    };

    struct ValidationConfig {
        double speedConsistencyThreshold;      // 5% tolerance
        double powerConservationThreshold;     // 10% tolerance
        double maxAccelerationRpmPerS;        // 600 RPM/s
        double brakePowerThreshold;            // 15% tolerance

        ValidationConfig()
            : speedConsistencyThreshold(0.05)
            , powerConservationThreshold(0.10)
            , maxAccelerationRpmPerS(600.0)
            , brakePowerThreshold(0.15)
        {}
    };

    /**
     * @brief Check speed consistency between torque sensor and encoder
     * R1: |speedTorque - speedEncoder| / max(speedTorque, speedEncoder) < threshold
     */
    static ValidationResult checkSpeedConsistency(
        const Domain::TelemetrySnapshot& snapshot,
        const ValidationConfig& config = ValidationConfig());

    /**
     * @brief Check acceleration limit
     * R2: |Δspeed / Δt| < maxAcceleration
     */
    static ValidationResult checkAccelerationLimit(
        const Domain::TelemetrySnapshot& current,
        const Domain::TelemetrySnapshot& previous,
        const ValidationConfig& config = ValidationConfig());

    /**
     * @brief Check power conservation
     * R4: |P_measured - P_calculated| / P_measured < threshold
     * P_calculated = T × ω × 2π/60
     */
    static ValidationResult checkPowerConservation(
        const Domain::TelemetrySnapshot& snapshot,
        const ValidationConfig& config = ValidationConfig());

    /**
     * @brief Check brake power dissipation
     * R7: P_brake = V × I (basic electrical power law)
     */
    static ValidationResult checkBrakePower(
        const Domain::TelemetrySnapshot& snapshot,
        const ValidationConfig& config = ValidationConfig());

    /**
     * @brief Validate all rules
     */
    static QVector<ValidationResult> validateAll(
        const Domain::TelemetrySnapshot& current,
        const Domain::TelemetrySnapshot& previous,
        const ValidationConfig& config = ValidationConfig());
};

} // namespace Validation
} // namespace Infrastructure

#endif // PHYSICSVALIDATOR_H
