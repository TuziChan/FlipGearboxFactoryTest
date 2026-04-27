#include "ConfigLoader.h"
#include "RecipeConfig.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QDebug>

namespace Infrastructure {
namespace Config {

ConfigLoader::ConfigLoader()
    : m_lastError()
{
}

bool ConfigLoader::loadAppConfig(const QString& filePath, AppConfig& config) {
    QJsonObject json;
    if (!readJsonFile(filePath, json)) {
        return false;
    }

    config.appName = json["appName"].toString(config.appName);
    config.version = json["version"].toString(config.version);
    config.logPath = json["logPath"].toString(config.logPath);
    config.reportPath = json["reportPath"].toString(config.reportPath);
    config.maxLogFiles = json["maxLogFiles"].toInt(config.maxLogFiles);
    config.enableDebugLog = json["enableDebugLog"].toBool(config.enableDebugLog);

    return true;
}

bool ConfigLoader::loadStationConfig(const QString& filePath, StationConfig& config) {
    QJsonObject json;
    if (!readJsonFile(filePath, json)) {
        return false;
    }

    config.stationId = json["stationId"].toString();
    config.stationName = json["stationName"].toString();
    config.brakeChannel = json["brakeChannel"].toInt(1);
    config.defaultRecipe = json["defaultRecipe"].toString();

    // Use config object values as fallback (already initialized by StationConfig constructor)
    // to ensure consistency with device manual specifications.

    QJsonObject aqmd = json["aqmd"].toObject();
    config.aqmdConfig.portName = aqmd["portName"].toString(config.aqmdConfig.portName);
    config.aqmdConfig.slaveId = static_cast<uint8_t>(aqmd["slaveId"].toInt(config.aqmdConfig.slaveId));
    config.aqmdConfig.baudRate = aqmd["baudRate"].toInt(config.aqmdConfig.baudRate);
    config.aqmdConfig.timeout = aqmd["timeout"].toInt(config.aqmdConfig.timeout);
    config.aqmdConfig.parity = aqmd["parity"].toString(config.aqmdConfig.parity);
    config.aqmdConfig.stopBits = aqmd["stopBits"].toInt(config.aqmdConfig.stopBits);
    config.aqmdConfig.enabled = aqmd["enabled"].toBool(config.aqmdConfig.enabled);
    config.aqmdConfig.pollIntervalUs = aqmd["pollIntervalUs"].toInt(config.aqmdConfig.pollIntervalUs);
    config.aqmdConfig.communicationMode = aqmd["communicationMode"].toInt(config.aqmdConfig.communicationMode);

    QJsonObject dyn200 = json["dyn200"].toObject();
    config.dyn200Config.portName = dyn200["portName"].toString(config.dyn200Config.portName);
    config.dyn200Config.slaveId = static_cast<uint8_t>(dyn200["slaveId"].toInt(config.dyn200Config.slaveId));
    config.dyn200Config.baudRate = dyn200["baudRate"].toInt(config.dyn200Config.baudRate);
    config.dyn200Config.timeout = dyn200["timeout"].toInt(config.dyn200Config.timeout);
    config.dyn200Config.parity = dyn200["parity"].toString(config.dyn200Config.parity);
    config.dyn200Config.stopBits = dyn200["stopBits"].toInt(config.dyn200Config.stopBits);
    config.dyn200Config.enabled = dyn200["enabled"].toBool(config.dyn200Config.enabled);
    config.dyn200Config.pollIntervalUs = dyn200["pollIntervalUs"].toInt(config.dyn200Config.pollIntervalUs);
    config.dyn200Config.communicationMode = dyn200["communicationMode"].toInt(config.dyn200Config.communicationMode);

    QJsonObject encoder = json["encoder"].toObject();
    config.encoderConfig.portName = encoder["portName"].toString(config.encoderConfig.portName);
    config.encoderConfig.slaveId = static_cast<uint8_t>(encoder["slaveId"].toInt(config.encoderConfig.slaveId));
    config.encoderConfig.baudRate = encoder["baudRate"].toInt(config.encoderConfig.baudRate);
    config.encoderConfig.timeout = encoder["timeout"].toInt(config.encoderConfig.timeout);
    config.encoderConfig.parity = encoder["parity"].toString(config.encoderConfig.parity);
    config.encoderConfig.stopBits = encoder["stopBits"].toInt(config.encoderConfig.stopBits);
    config.encoderConfig.encoderResolution = static_cast<uint16_t>(encoder["resolution"].toInt(config.encoderConfig.encoderResolution));
    config.encoderConfig.enabled = encoder["enabled"].toBool(config.encoderConfig.enabled);
    config.encoderConfig.pollIntervalUs = encoder["pollIntervalUs"].toInt(config.encoderConfig.pollIntervalUs);
    config.encoderConfig.communicationMode = encoder["communicationMode"].toInt(config.encoderConfig.communicationMode);

    QJsonObject brake = json["brake"].toObject();
    config.brakeConfig.portName = brake["portName"].toString(config.brakeConfig.portName);
    config.brakeConfig.slaveId = static_cast<uint8_t>(brake["slaveId"].toInt(config.brakeConfig.slaveId));
    config.brakeConfig.baudRate = brake["baudRate"].toInt(config.brakeConfig.baudRate);
    config.brakeConfig.timeout = brake["timeout"].toInt(config.brakeConfig.timeout);
    config.brakeConfig.parity = brake["parity"].toString(config.brakeConfig.parity);
    config.brakeConfig.stopBits = brake["stopBits"].toInt(config.brakeConfig.stopBits);
    config.brakeConfig.enabled = brake["enabled"].toBool(config.brakeConfig.enabled);
    config.brakeConfig.pollIntervalUs = brake["pollIntervalUs"].toInt(config.brakeConfig.pollIntervalUs);
    config.brakeConfig.communicationMode = brake["communicationMode"].toInt(config.brakeConfig.communicationMode);
    config.brakeChannel = brake["channel"].toInt(config.brakeChannel);

    return true;
}

bool ConfigLoader::loadRecipe(const QString& filePath, Domain::TestRecipe& recipe) {
    QJsonObject json;
    if (!readJsonFile(filePath, json)) {
        return false;
    }

    recipe = RecipeConfig::fromJson(json);
    return true;
}

bool ConfigLoader::loadRecipes(const QString& dirPath, QMap<QString, Domain::TestRecipe>& recipes) {
    QDir dir(dirPath);
    if (!dir.exists()) {
        m_lastError = QString("Recipe directory does not exist: %1").arg(dirPath);
        return false;
    }

    QStringList filters;
    filters << "*.json";
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files);

    for (const QFileInfo& fileInfo : files) {
        Domain::TestRecipe recipe;
        if (loadRecipe(fileInfo.absoluteFilePath(), recipe)) {
            recipes[recipe.name] = recipe;
            qDebug() << "Loaded recipe:" << recipe.name;
        } else {
            qWarning() << "Failed to load recipe from:" << fileInfo.fileName() << "-" << m_lastError;
        }
    }

    return !recipes.isEmpty();
}

bool ConfigLoader::saveRecipe(const QString& filePath, const Domain::TestRecipe& recipe) {
    QJsonObject json = RecipeConfig::toJson(recipe);
    return writeJsonFile(filePath, json);
}

bool ConfigLoader::readJsonFile(const QString& filePath, QJsonObject& json) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_lastError = QString("Failed to open file: %1").arg(filePath);
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        m_lastError = QString("JSON parse error: %1").arg(parseError.errorString());
        return false;
    }

    if (!doc.isObject()) {
        m_lastError = "JSON document is not an object";
        return false;
    }

    json = doc.object();
    return true;
}

bool ConfigLoader::writeJsonFile(const QString& filePath, const QJsonObject& json) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = QString("Failed to open file for writing: %1").arg(filePath);
        return false;
    }

    QJsonDocument doc(json);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    return true;
}

} // namespace Config
} // namespace Infrastructure
