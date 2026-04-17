#include "DeviceConfigService.h"
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariantHash>
#include <QDir>

namespace Infrastructure {
namespace Config {

DeviceConfigService::DeviceConfigService(const QString& stationConfigPath,
                                         const StationConfig& defaults,
                                         QObject* parent)
    : QObject(parent)
    , m_stationConfigPath(stationConfigPath)
    , m_defaultConfig(defaults)
    , m_lastError()
{
}

QVariantMap DeviceConfigService::defaultDeviceConfig() const {
    auto dc = [](const Infrastructure::Config::DeviceConfig& c) -> QVariantMap {
        return {
            {"portName", c.portName.isEmpty() ? QString() : c.portName},
            {"baudRate", c.baudRate},
            {"slaveId", static_cast<int>(c.slaveId)},
            {"timeout", c.timeout},
            {"parity", c.parity},
            {"stopBits", c.stopBits},
            {"enabled", c.enabled},
            {"pollIntervalUs", c.pollIntervalUs}
        };
    };

    auto encoderMap = dc(m_defaultConfig.encoderConfig);
    encoderMap["resolution"] = static_cast<int>(m_defaultConfig.encoderConfig.encoderResolution);

    auto brakeMap = dc(m_defaultConfig.brakeConfig);
    brakeMap["channel"] = m_defaultConfig.brakeConfig.enabled ? 1 : 0;

    return {
        {"aqmd", dc(m_defaultConfig.aqmdConfig)},
        {"dyn200", dc(m_defaultConfig.dyn200Config)},
        {"encoder", encoderMap},
        {"brake", brakeMap}
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
        if (deviceJson.contains("pollIntervalUs")) deviceMap["pollIntervalUs"] = deviceJson.value("pollIntervalUs").toInt(deviceMap.value("pollIntervalUs").toInt());
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
        if (map.contains("pollIntervalUs")) obj["pollIntervalUs"] = map.value("pollIntervalUs").toInt();
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
