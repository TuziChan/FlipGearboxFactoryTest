#include "RuntimeManager.h"
#include "StationRuntimeFactory.h"
#include "ConfigLoader.h"
#include <QDebug>
#include <QMutexLocker>

namespace Infrastructure {
namespace Config {

RuntimeManager::RuntimeManager(const StationConfig& config, const QString& configPath, QObject* parent)
    : QObject(parent)
    , m_config(config)
    , m_configPath(configPath)
{
    recreateRuntime();
}

void RuntimeManager::recreateRuntime() {
    qDebug() << "Creating new runtime...";
    auto newRuntime = StationRuntimeFactory::create(m_config);

    if (newRuntime) {
        qDebug() << "Initializing new runtime...";
        if (!newRuntime->initialize()) {
            qWarning() << "Failed to initialize runtime:" << newRuntime->lastError();
        } else {
            qDebug() << "Runtime initialized successfully";
        }
    }

    {
        QMutexLocker locker(&m_mutex);
        if (m_runtime) {
            qDebug() << "Shutting down old runtime...";
            m_runtime->shutdown();
        }
        m_runtime = std::move(newRuntime);
    }
}

void RuntimeManager::reloadRuntime() {
    qDebug() << "Reloading runtime with current config from" << m_configPath;

    StationConfig updatedConfig = m_config;
    ConfigLoader configLoader;
    if (configLoader.loadStationConfig(m_configPath, updatedConfig)) {
        m_config = updatedConfig;
        qDebug() << "Config reloaded from disk";
    } else {
        qWarning() << "Failed to reload config from" << m_configPath << ", using cached config";
    }

    recreateRuntime();
    emit runtimeRecreated(m_runtime.get());
}

void RuntimeManager::disconnectDeviceBus(const QString& deviceKey) {
    QMutexLocker locker(&m_mutex);
    if (!m_runtime) return;

    Bus::IBusController* bus = nullptr;
    if (deviceKey == QStringLiteral("aqmd")) bus = m_runtime->aqmdBus();
    else if (deviceKey == QStringLiteral("dyn200")) bus = m_runtime->dyn200Bus();
    else if (deviceKey == QStringLiteral("encoder")) bus = m_runtime->encoderBus();
    else if (deviceKey == QStringLiteral("brake")) bus = m_runtime->brakeBus();

    if (bus && bus->isOpen()) {
        qDebug() << "Disconnecting bus for" << deviceKey;
        bus->close();
    }
}

} // namespace Config
} // namespace Infrastructure
