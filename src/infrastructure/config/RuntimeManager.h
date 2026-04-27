#ifndef RUNTIMEMANAGER_H
#define RUNTIMEMANAGER_H

#include <QObject>
#include <QMutex>
#include <memory>
#include <QString>
#include "StationRuntime.h"
#include "StationConfig.h"

namespace Infrastructure {
namespace Config {

/**
 * @brief Manages StationRuntime lifecycle
 */
class RuntimeManager : public QObject {
    Q_OBJECT

public:
    explicit RuntimeManager(const StationConfig& config, const QString& configPath, QObject* parent = nullptr);
    ~RuntimeManager() override = default;

    StationRuntime* runtime() const {
        QMutexLocker locker(&m_mutex);
        return m_runtime.get();
    }

    Q_INVOKABLE void reloadRuntime();
    Q_INVOKABLE void disconnectDeviceBus(const QString& deviceKey);

signals:
    void runtimeRecreated(StationRuntime* newRuntime);

private:
    StationConfig m_config;
    QString m_configPath;
    std::unique_ptr<StationRuntime> m_runtime;
    mutable QMutex m_mutex;

    void recreateRuntime();
};

} // namespace Config
} // namespace Infrastructure

#endif // RUNTIMEMANAGER_H
