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

    QJsonObject aqmd = json["aqmd"].toObject();
    config.aqmdConfig.portName = aqmd["portName"].toString();
    config.aqmdConfig.slaveId = static_cast<uint8_t>(aqmd["slaveId"].toInt(1));
    config.aqmdConfig.baudRate = aqmd["baudRate"].toInt(9600);
    config.aqmdConfig.timeout = aqmd["timeout"].toInt(500);
    config.aqmdConfig.parity = aqmd["parity"].toString("None");
    config.aqmdConfig.stopBits = aqmd["stopBits"].toInt(1);
    config.aqmdConfig.enabled = aqmd["enabled"].toBool(true);

    QJsonObject dyn200 = json["dyn200"].toObject();
    config.dyn200Config.portName = dyn200["portName"].toString();
    config.dyn200Config.slaveId = static_cast<uint8_t>(dyn200["slaveId"].toInt(2));
    config.dyn200Config.baudRate = dyn200["baudRate"].toInt(9600);
    config.dyn200Config.timeout = dyn200["timeout"].toInt(300);
    config.dyn200Config.parity = dyn200["parity"].toString("None");
    config.dyn200Config.stopBits = dyn200["stopBits"].toInt(1);
    config.dyn200Config.enabled = dyn200["enabled"].toBool(true);

    QJsonObject encoder = json["encoder"].toObject();
    config.encoderConfig.portName = encoder["portName"].toString();
    config.encoderConfig.slaveId = static_cast<uint8_t>(encoder["slaveId"].toInt(3));
    config.encoderConfig.baudRate = encoder["baudRate"].toInt(9600);
    config.encoderConfig.timeout = encoder["timeout"].toInt(200);
    config.encoderConfig.parity = encoder["parity"].toString("None");
    config.encoderConfig.stopBits = encoder["stopBits"].toInt(1);
    config.encoderConfig.encoderResolution = static_cast<uint16_t>(encoder["resolution"].toInt(4096));
    config.encoderConfig.enabled = encoder["enabled"].toBool(true);

    QJsonObject brake = json["brake"].toObject();
    config.brakeConfig.portName = brake["portName"].toString();
    config.brakeConfig.slaveId = static_cast<uint8_t>(brake["slaveId"].toInt(4));
    config.brakeConfig.baudRate = brake["baudRate"].toInt(9600);
    config.brakeConfig.timeout = brake["timeout"].toInt(500);
    config.brakeConfig.parity = brake["parity"].toString("None");
    config.brakeConfig.stopBits = brake["stopBits"].toInt(1);
    config.brakeConfig.enabled = brake["enabled"].toBool(true);
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
