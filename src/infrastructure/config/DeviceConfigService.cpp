#include "DeviceConfigService.h"
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariantHash>
#include <QDir>

namespace Infrastructure {
namespace Config {

DeviceConfigService::DeviceConfigService(const QString& stationConfigPath, QObject* parent)
    : QObject(parent)
    , m_stationConfigPath(stationConfigPath)
    , m_lastError()
{
}

QVariantMap DeviceConfigService::defaultDeviceConfig() const {
    return {
        {"aqmd", QVariantMap{{"portName", "COM3"}, {"baudRate", 9600}, {"slaveId", 1}, {"timeout", 1000}, {"parity", "None"}, {"stopBits", 1}, {"enabled", true}}},
        {"dyn200", QVariantMap{{"portName", "COM4"}, {"baudRate", 9600}, {"slaveId", 2}, {"timeout", 1000}, {"parity", "None"}, {"stopBits", 1}, {"enabled", true}}},
        {"encoder", QVariantMap{{"portName", "COM5"}, {"baudRate", 9600}, {"slaveId", 3}, {"timeout", 1000}, {"parity", "None"}, {"stopBits", 1}, {"resolution", 4096}, {"enabled", true}}},
        {"brake", QVariantMap{{"portName", "COM6"}, {"baudRate", 9600}, {"slaveId", 4}, {"timeout", 1000}, {"parity", "None"}, {"stopBits", 1}, {"channel", 1}, {"enabled", true}}}
    };
}

QVariantMap DeviceConfigService::loadDeviceConfig() const {
    QFile file(m_stationConfigPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_lastError = QString("Failed to open station config: %1").arg(m_stationConfigPath);
        return defaultDeviceConfig();
    }

    QJsonParseError parseError;
    const auto doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    file.close();

    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        m_lastError = QString("Failed to parse station config: %1").arg(parseError.errorString());
        return defaultDeviceConfig();
    }

    const auto json = doc.object();
    auto result = defaultDeviceConfig();
    const QStringList deviceKeys = {"aqmd", "dyn200", "encoder", "brake"};
    for (const auto& key : deviceKeys) {
        const auto deviceJson = json.value(key).toObject();
        auto deviceMap = result.value(key).toMap();
        if (deviceJson.contains("portName")) deviceMap["portName"] = deviceJson.value("portName").toString(deviceMap.value("portName").toString());
        if (deviceJson.contains("baudRate")) deviceMap["baudRate"] = deviceJson.value("baudRate").toInt(deviceMap.value("baudRate").toInt());
        if (deviceJson.contains("slaveId")) deviceMap["slaveId"] = deviceJson.value("slaveId").toInt(deviceMap.value("slaveId").toInt());
        if (deviceJson.contains("timeout")) deviceMap["timeout"] = deviceJson.value("timeout").toInt(deviceMap.value("timeout").toInt());
        if (deviceJson.contains("parity")) deviceMap["parity"] = deviceJson.value("parity").toString(deviceMap.value("parity").toString());
        if (deviceJson.contains("stopBits")) deviceMap["stopBits"] = deviceJson.value("stopBits").toInt(deviceMap.value("stopBits").toInt());
        if (deviceJson.contains("enabled")) deviceMap["enabled"] = deviceJson.value("enabled").toBool(deviceMap.value("enabled").toBool());
        if (key == "encoder" && deviceJson.contains("resolution")) deviceMap["resolution"] = deviceJson.value("resolution").toInt(deviceMap.value("resolution").toInt());
        if (key == "brake" && deviceJson.contains("channel")) deviceMap["channel"] = deviceJson.value("channel").toInt(deviceMap.value("channel").toInt());
        result[key] = deviceMap;
    }
    m_lastError.clear();
    return result;
}

bool DeviceConfigService::saveDeviceConfig(const QVariantMap& deviceConfig) {
    QFile file(m_stationConfigPath);
    QJsonObject rootObject;
    if (file.exists()) {
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            m_lastError = QString("Failed to open station config: %1").arg(m_stationConfigPath);
            return false;
        }
        QJsonParseError parseError;
        const auto doc = QJsonDocument::fromJson(file.readAll(), &parseError);
        file.close();
        if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
            rootObject = doc.object();
        }
    }

    const QStringList deviceKeys = {"aqmd", "dyn200", "encoder", "brake"};
    for (const auto& key : deviceKeys) {
        const auto map = deviceConfig.value(key).toMap();
        QJsonObject obj;
        obj["portName"] = map.value("portName").toString();
        obj["baudRate"] = map.value("baudRate").toInt();
        obj["slaveId"] = map.value("slaveId").toInt();
        obj["timeout"] = map.value("timeout").toInt();
        obj["parity"] = map.value("parity").toString();
        obj["stopBits"] = map.value("stopBits").toInt();
        obj["enabled"] = map.value("enabled", true).toBool();
        if (key == "encoder") obj["resolution"] = map.value("resolution").toInt();
        if (key == "brake") {
            obj["channel"] = map.value("channel", 1).toInt();
            rootObject["brakeChannel"] = map.value("channel", 1).toInt();
        }
        rootObject[key] = obj;
    }

    const auto parentDir = QFileInfo(m_stationConfigPath).absoluteDir();
    if (!parentDir.exists()) {
        QDir().mkpath(parentDir.absolutePath());
    }

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        m_lastError = QString("Failed to write station config: %1").arg(m_stationConfigPath);
        return false;
    }
    file.write(QJsonDocument(rootObject).toJson(QJsonDocument::Indented));
    file.close();
    m_lastError.clear();
    return true;
}

QString DeviceConfigService::lastError() const {
    return m_lastError;
}

QString DeviceConfigService::configPath() const {
    return m_stationConfigPath;
}

} // namespace Config
} // namespace Infrastructure
