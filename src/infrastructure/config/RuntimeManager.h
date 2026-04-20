#ifndef RUNTIMEMANAGER_H
#define RUNTIMEMANAGER_H

#include <QObject>
#include <memory>
#include "StationRuntime.h"
#include "StationConfig.h"

namespace Infrastructure {
namespace Config {

/**
 * @brief Manages StationRuntime lifecycle and mode switching
 */
class RuntimeManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isMockMode READ isMockMode NOTIFY mockModeChanged)

public:
    explicit RuntimeManager(const StationConfig& config, bool initialMockMode, QObject* parent = nullptr);
    ~RuntimeManager() override = default;

    StationRuntime* runtime() const { return m_runtime.get(); }
    bool isMockMode() const { return m_isMockMode; }

    Q_INVOKABLE void switchMode(bool mockMode);

signals:
    void mockModeChanged();
    void runtimeRecreated(StationRuntime* newRuntime);

private:
    StationConfig m_config;
    std::unique_ptr<StationRuntime> m_runtime;
    bool m_isMockMode;

    void recreateRuntime(bool mockMode);
};

} // namespace Config
} // namespace Infrastructure

#endif // RUNTIMEMANAGER_H
