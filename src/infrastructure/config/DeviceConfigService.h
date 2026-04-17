#ifndef DEVICECONFIGSERVICE_H
#define DEVICECONFIGSERVICE_H

#include <QObject>
#include <QVariantMap>
#include <QString>
#include "StationConfig.h"

namespace Infrastructure {
namespace Config {

class DeviceConfigService : public QObject {
    Q_OBJECT
public:
    explicit DeviceConfigService(const QString& stationConfigPath,
                                 const StationConfig& defaults = StationConfig(),
                                 QObject* parent = nullptr);

    Q_INVOKABLE QVariantMap loadDeviceConfig() const;
    Q_INVOKABLE bool saveDeviceConfig(const QVariantMap& deviceConfig);
    Q_INVOKABLE QString lastError() const;
    Q_INVOKABLE QString configPath() const;

private:
    QString m_stationConfigPath;
    StationConfig m_defaultConfig;
    mutable QString m_lastError;

    QVariantMap defaultDeviceConfig() const;
};

} // namespace Config
} // namespace Infrastructure

#endif // DEVICECONFIGSERVICE_H
