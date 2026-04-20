#include "RuntimeManager.h"
#include "StationRuntimeFactory.h"
#include <QDebug>

namespace Infrastructure {
namespace Config {

RuntimeManager::RuntimeManager(const StationConfig& config, bool initialMockMode, QObject* parent)
    : QObject(parent)
    , m_config(config)
    , m_isMockMode(initialMockMode)
{
    recreateRuntime(initialMockMode);
}

void RuntimeManager::switchMode(bool mockMode) {
    if (m_isMockMode == mockMode) {
        qDebug() << "Already in" << (mockMode ? "mock" : "real") << "mode, no switch needed";
        return;
    }

    qDebug() << "Switching from" << (m_isMockMode ? "mock" : "real")
             << "to" << (mockMode ? "mock" : "real") << "mode";

    recreateRuntime(mockMode);
    m_isMockMode = mockMode;

    emit mockModeChanged();
    emit runtimeRecreated(m_runtime.get());
}

void RuntimeManager::recreateRuntime(bool mockMode) {
    // Shutdown old runtime if exists
    if (m_runtime) {
        qDebug() << "Shutting down old runtime...";
        m_runtime->shutdown();
        m_runtime.reset();
    }

    // Create new runtime
    qDebug() << "Creating new runtime in" << (mockMode ? "mock" : "real") << "mode...";
    m_runtime = StationRuntimeFactory::create(m_config, mockMode);

    // Initialize the new runtime
    if (m_runtime) {
        qDebug() << "Initializing new runtime...";
        if (!m_runtime->initialize()) {
            qWarning() << "Failed to initialize runtime:" << m_runtime->lastError();
        } else {
            qDebug() << "Runtime initialized successfully";
        }
    }
}

} // namespace Config
} // namespace Infrastructure
