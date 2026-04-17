#ifndef RECIPECONFIG_H
#define RECIPECONFIG_H

#include "../../domain/TestRecipe.h"
#include <QString>
#include <QJsonObject>

namespace Infrastructure {
namespace Config {

/**
 * @brief Recipe configuration loader/saver
 */
class RecipeConfig {
public:
    /**
     * @brief Load recipe from JSON object
     */
    static Domain::TestRecipe fromJson(const QJsonObject& json);
    
    /**
     * @brief Save recipe to JSON object
     */
    static QJsonObject toJson(const Domain::TestRecipe& recipe);
    
    /**
     * @brief Create default recipe for testing
     */
    static Domain::TestRecipe createDefault();
};

} // namespace Config
} // namespace Infrastructure

#endif // RECIPECONFIG_H
