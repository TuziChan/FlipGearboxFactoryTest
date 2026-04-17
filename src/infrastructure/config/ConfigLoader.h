#ifndef CONFIGLOADER_H
#define CONFIGLOADER_H

#include "AppConfig.h"
#include "StationConfig.h"
#include "../../domain/TestRecipe.h"
#include <QString>
#include <QJsonObject>
#include <QMap>

namespace Infrastructure {
namespace Config {

/**
 * @brief Configuration loader from JSON files
 */
class ConfigLoader {
public:
    ConfigLoader();
    
    /**
     * @brief Load application configuration
     */
    bool loadAppConfig(const QString& filePath, AppConfig& config);
    
    /**
     * @brief Load station configuration
     */
    bool loadStationConfig(const QString& filePath, StationConfig& config);
    
    /**
     * @brief Load recipe from file
     */
    bool loadRecipe(const QString& filePath, Domain::TestRecipe& recipe);
    
    /**
     * @brief Load all recipes from directory
     */
    bool loadRecipes(const QString& dirPath, QMap<QString, Domain::TestRecipe>& recipes);
    
    /**
     * @brief Save recipe to file
     */
    bool saveRecipe(const QString& filePath, const Domain::TestRecipe& recipe);
    
    /**
     * @brief Get last error message
     */
    QString lastError() const { return m_lastError; }

private:
    QString m_lastError;
    
    bool readJsonFile(const QString& filePath, QJsonObject& json);
    bool writeJsonFile(const QString& filePath, const QJsonObject& json);
};

} // namespace Config
} // namespace Infrastructure

#endif // CONFIGLOADER_H
