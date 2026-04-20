#ifndef RECIPEVALIDATOR_H
#define RECIPEVALIDATOR_H

#include "TestRecipe.h"
#include <QString>
#include <QStringList>

namespace Domain {

/**
 * @brief Validates test recipe parameters for safety and correctness
 */
class RecipeValidator {
public:
    /**
     * @brief Validate all recipe parameters
     * @param recipe Recipe to validate
     * @param errors Output list of validation errors
     * @return true if valid, false otherwise
     */
    static bool validate(const TestRecipe& recipe, QStringList& errors);

private:
    // Parameter range constants
    static constexpr double MIN_DUTY_CYCLE = 0.0;
    static constexpr double MAX_DUTY_CYCLE = 100.0;
    static constexpr double MIN_ANGLE = -360.0;
    static constexpr double MAX_ANGLE = 360.0;
    static constexpr double MIN_TOLERANCE = 0.0;
    static constexpr double MAX_TOLERANCE = 180.0;
    static constexpr int MIN_TIMEOUT_MS = 100;
    static constexpr int MAX_TIMEOUT_MS = 600000; // 10 minutes
    static constexpr int MIN_DURATION_MS = 0;
    static constexpr int MAX_DURATION_MS = 300000; // 5 minutes
    static constexpr double MIN_CURRENT = 0.0;
    static constexpr double MAX_CURRENT = 50.0; // 50A max
    static constexpr double MIN_VOLTAGE = 0.0;
    static constexpr double MAX_VOLTAGE = 100.0; // 100V max
    static constexpr double MIN_SPEED = 0.0;
    static constexpr double MAX_SPEED = 10000.0; // 10000 RPM max
    static constexpr double MIN_TORQUE = -1000.0;
    static constexpr double MAX_TORQUE = 1000.0; // ±1000 Nm max

    static bool validateRange(double value, double min, double max, 
                             const QString& paramName, QStringList& errors);
    static bool validateRange(int value, int min, int max, 
                             const QString& paramName, QStringList& errors);
    static bool validatePositive(double value, const QString& paramName, QStringList& errors);
    static bool validatePositive(int value, const QString& paramName, QStringList& errors);
};

} // namespace Domain

#endif // RECIPEVALIDATOR_H
