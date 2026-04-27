#ifndef TEAMMONITORSERVICE_H
#define TEAMMONITORSERVICE_H

#include <QObject>
#include <QTimer>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QVector>
#include "TeamRoleStatus.h"
#include "ITeamDataProvider.h"

namespace TeamOps {

/**
 * @brief Background service that polls team status at regular intervals
 *
 * Runs on its own QThread to avoid blocking the UI thread.
 * Uses ITeamDataProvider abstraction to support multiple data sources
 * (MCP tools and files).
 *
 * @note All data access is protected by QMutex for thread safety.
 */
class TeamMonitorService : public QObject {
    Q_OBJECT

public:
    explicit TeamMonitorService(ITeamDataProvider* provider,
                                 int pollIntervalMs = 5000,
                                 QObject* parent = nullptr);
    ~TeamMonitorService() override;

    /**
     * @brief Start monitoring in background thread
     */
    void startMonitoring();

    /**
     * @brief Stop monitoring and clean up thread
     */
    void stopMonitoring();

    /**
     * @brief Check if monitoring is active
     */
    bool isMonitoring() const;

    /**
     * @brief Change polling interval
     */
    void setPollInterval(int ms);
    int pollInterval() const { return m_pollIntervalMs; }

    /**
     * @brief Get current role statuses (thread-safe copy)
     */
    QVector<TeamRoleStatus> roleStatuses() const;

    /**
     * @brief Get current pipeline items (thread-safe copy)
     */
    QVector<TaskPipelineItem> pipelineItems() const;

    /**
     * @brief Force an immediate refresh
     */
    Q_INVOKABLE void refreshNow();

    /**
     * @brief Get last error from data provider
     */
    QString lastError() const;

    /**
     * @brief Number of successful refreshes
     */
    int refreshCount() const;

    /**
     * @brief Time of last successful refresh
     */
    QDateTime lastRefreshTime() const;

signals:
    /**
     * @brief Emitted when role statuses are updated
     */
    void roleStatusesUpdated(const QVector<TeamRoleStatus>& statuses);

    /**
     * @brief Emitted when pipeline items are updated
     */
    void pipelineItemsUpdated(const QVector<TaskPipelineItem>& items);

    /**
     * @brief Emitted on any data update (convenience signal)
     */
    void dataUpdated();

    /**
     * @brief Emitted when refresh fails
     */
    void refreshFailed(const QString& error);

    /**
     * @brief Emitted when monitoring starts/stops
     */
    void monitoringStateChanged(bool active);

private slots:
    void onPollTick();
    void onProviderDataChanged();

private:
    ITeamDataProvider* m_provider;
    QThread* m_workerThread;
    QTimer* m_pollTimer;
    int m_pollIntervalMs;
    bool m_isMonitoring;

    // Thread-safe cached data
    mutable QMutex m_dataMutex;
    QVector<TeamRoleStatus> m_cachedRoleStatuses;
    QVector<TaskPipelineItem> m_cachedPipelineItems;
    QString m_lastError;
    int m_refreshCount;
    QDateTime m_lastRefreshTime;

    void updateData();
};

} // namespace TeamOps

#endif // TEAMMONITORSERVICE_H
