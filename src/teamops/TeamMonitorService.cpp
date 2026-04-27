#include "TeamMonitorService.h"
#include <QDebug>

namespace TeamOps {

TeamMonitorService::TeamMonitorService(ITeamDataProvider* provider,
                                        int pollIntervalMs,
                                        QObject* parent)
    : QObject(parent)
    , m_provider(provider)
    , m_workerThread(new QThread(this))
    , m_pollTimer(new QTimer())
    , m_pollIntervalMs(pollIntervalMs)
    , m_isMonitoring(false)
    , m_refreshCount(0)
{
    Q_ASSERT(provider != nullptr);

    // Move timer to worker thread
    m_pollTimer->moveToThread(m_workerThread);

    // Timer runs in worker thread
    connect(m_pollTimer, &QTimer::timeout,
            this, &TeamMonitorService::onPollTick, Qt::QueuedConnection);

    // Connect provider data change signal
    connect(m_provider, &ITeamDataProvider::dataChanged,
            this, &TeamMonitorService::onProviderDataChanged, Qt::QueuedConnection);
}

TeamMonitorService::~TeamMonitorService()
{
    stopMonitoring();

    if (m_pollTimer) {
        if (m_workerThread->isRunning()) {
            QTimer* timer = m_pollTimer;
            const bool stoppedAndDeleted = QMetaObject::invokeMethod(
                timer,
                [timer]() {
                    timer->stop();
                    delete timer;
                },
                Qt::BlockingQueuedConnection);

            if (!stoppedAndDeleted) {
                m_pollTimer->stop();
                delete m_pollTimer;
            }
        } else {
            m_pollTimer->stop();
            delete m_pollTimer;
        }

        m_pollTimer = nullptr;
    }

    if (m_workerThread->isRunning()) {
        m_workerThread->quit();
        m_workerThread->wait(3000);
    }
}

void TeamMonitorService::startMonitoring()
{
    if (m_isMonitoring) return;

    if (!m_workerThread->isRunning()) {
        m_workerThread->start(QThread::LowPriority);
    }

    // Start timer in worker thread context via queued invocation
    QMetaObject::invokeMethod(m_pollTimer, [this]() {
        m_pollTimer->start(m_pollIntervalMs);
    }, Qt::QueuedConnection);

    // Do an immediate refresh
    refreshNow();

    m_isMonitoring = true;
    emit monitoringStateChanged(true);
    qDebug() << "[TeamMonitorService] Monitoring started, interval:" << m_pollIntervalMs << "ms";
}

void TeamMonitorService::stopMonitoring()
{
    if (!m_isMonitoring) return;

    if (m_pollTimer) {
        if (m_workerThread->isRunning()) {
            QMetaObject::invokeMethod(m_pollTimer, [timer = m_pollTimer]() {
                timer->stop();
            }, Qt::BlockingQueuedConnection);
        } else {
            m_pollTimer->stop();
        }
    }

    m_isMonitoring = false;
    emit monitoringStateChanged(false);
    qDebug() << "[TeamMonitorService] Monitoring stopped";
}

bool TeamMonitorService::isMonitoring() const
{
    return m_isMonitoring;
}

void TeamMonitorService::setPollInterval(int ms)
{
    if (ms < 1000) ms = 1000; // Minimum 1 second
    m_pollIntervalMs = ms;

    if (m_isMonitoring) {
        QMetaObject::invokeMethod(m_pollTimer, [this]() {
            m_pollTimer->setInterval(m_pollIntervalMs);
        }, Qt::QueuedConnection);
    }
}

QVector<TeamRoleStatus> TeamMonitorService::roleStatuses() const
{
    QMutexLocker locker(&m_dataMutex);
    return m_cachedRoleStatuses;
}

QVector<TaskPipelineItem> TeamMonitorService::pipelineItems() const
{
    QMutexLocker locker(&m_dataMutex);
    return m_cachedPipelineItems;
}

void TeamMonitorService::refreshNow()
{
    // Use QueuedConnection to ensure execution in worker thread context
    QMetaObject::invokeMethod(this, &TeamMonitorService::onPollTick, Qt::QueuedConnection);
}

QString TeamMonitorService::lastError() const
{
    QMutexLocker locker(&m_dataMutex);
    return m_lastError;
}

int TeamMonitorService::refreshCount() const
{
    QMutexLocker locker(&m_dataMutex);
    return m_refreshCount;
}

QDateTime TeamMonitorService::lastRefreshTime() const
{
    QMutexLocker locker(&m_dataMutex);
    return m_lastRefreshTime;
}

void TeamMonitorService::onPollTick()
{
    if (!m_provider || !m_provider->isAvailable()) {
        QMutexLocker locker(&m_dataMutex);
        m_lastError = QStringLiteral("Data provider not available");
        emit refreshFailed(m_lastError);
        return;
    }

    updateData();
}

void TeamMonitorService::onProviderDataChanged()
{
    // Provider detected external changes, refresh immediately
    updateData();
}

void TeamMonitorService::updateData()
{
    // Fetch from provider
    const QVector<TeamRoleStatus> roles = m_provider->fetchRoleStatuses();
    const QVector<TaskPipelineItem> pipeline = m_provider->fetchPipelineItems();

    // Update cached data under lock
    {
        QMutexLocker locker(&m_dataMutex);
        m_cachedRoleStatuses = roles;
        m_cachedPipelineItems = pipeline;
        m_refreshCount++;
        m_lastRefreshTime = QDateTime::currentDateTime();

        if (!m_provider->lastError().isEmpty()) {
            m_lastError = m_provider->lastError();
        } else {
            m_lastError.clear();
        }
    }

    // Emit signals (data is copied for thread safety)
    emit roleStatusesUpdated(roles);
    emit pipelineItemsUpdated(pipeline);
    emit dataUpdated();

    qDebug() << "[TeamMonitorService] Data refreshed, roles:" << roles.size()
             << "pipeline:" << pipeline.size();
}

} // namespace TeamOps
