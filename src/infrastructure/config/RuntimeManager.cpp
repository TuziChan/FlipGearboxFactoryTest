#include "RuntimeManager.h"
#include "StationRuntimeFactory.h"
#include <QDebug>
#include <QMutexLocker>

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
    // Create new runtime first
    qDebug() << "Creating new runtime in" << (mockMode ? "mock" : "real") << "mode...";
    auto newRuntime = StationRuntimeFactory::create(m_config, mockMode);

    // Initialize the new runtime
    if (newRuntime) {
        qDebug() << "Initializing new runtime...";
        if (!newRuntime->initialize()) {
            qWarning() << "Failed to initialize runtime:" << newRuntime->lastError();
        } else {
            qDebug() << "Runtime initialized successfully";
        }
    }

    // Critical section: shutdown old runtime and swap atomically
    {
        QMutexLocker locker(&m_mutex);

        // Shutdown old runtime if exists (after new one is ready)
        if (m_runtime) {
            qDebug() << "Shutting down old runtime...";
            m_runtime->shutdown();
        }

        // Atomic swap: replace old runtime with new one
        m_runtime = std::move(newRuntime);
    }
}

} // namespace Config
} // namespace Infrastructure
