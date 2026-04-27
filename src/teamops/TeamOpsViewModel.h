#ifndef TEAMOPSVIEWMODEL_H
#define TEAMOPSVIEWMODEL_H

#include <QObject>
#include <QVariantList>
#include <QVariantMap>
#include <QTimer>
#include "TeamMonitorService.h"

namespace TeamOps {

/**
 * @brief QML-exposed ViewModel for the TeamOps monitoring panel
 *
 * Bridges the background TeamMonitorService with QML UI bindings.
 * All properties are updated automatically when the service refreshes.
 */
class TeamOpsViewModel : public QObject {
    Q_OBJECT

    // Role status properties
    Q_PROPERTY(QVariantList roleStatuses READ roleStatuses NOTIFY roleStatusesChanged)
    Q_PROPERTY(QVariantList roleStatusCards READ roleStatusCards NOTIFY roleStatusCardsChanged)
    Q_PROPERTY(int totalRoleCount READ totalRoleCount NOTIFY roleStatusesChanged)
    Q_PROPERTY(int activeRoleCount READ activeRoleCount NOTIFY roleStatusesChanged)
    Q_PROPERTY(int idleRoleCount READ idleRoleCount NOTIFY roleStatusesChanged)
    Q_PROPERTY(int blockedRoleCount READ blockedRoleCount NOTIFY roleStatusesChanged)
    Q_PROPERTY(int offlineRoleCount READ offlineRoleCount NOTIFY roleStatusesChanged)

    // Pipeline properties
    Q_PROPERTY(QVariantList pipelineItems READ pipelineItems NOTIFY pipelineItemsChanged)
    Q_PROPERTY(int activeTaskCount READ activeTaskCount NOTIFY pipelineItemsChanged)
    Q_PROPERTY(int blockedTaskCount READ blockedTaskCount NOTIFY pipelineItemsChanged)
    Q_PROPERTY(int completedTaskCount READ completedTaskCount NOTIFY pipelineItemsChanged)

    // Monitoring state
    Q_PROPERTY(bool monitoring READ monitoring NOTIFY monitoringChanged)
    Q_PROPERTY(QString lastUpdateTime READ lastUpdateTime NOTIFY lastUpdateTimeChanged)
    Q_PROPERTY(int refreshCount READ refreshCount NOTIFY refreshCountChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    explicit TeamOpsViewModel(TeamMonitorService* monitorService,
                               QObject* parent = nullptr);
    ~TeamOpsViewModel() override = default;

    // Role status accessors
    QVariantList roleStatuses() const { return m_roleStatuses; }
    QVariantList roleStatusCards() const { return m_roleStatusCards; }
    int totalRoleCount() const { return m_roleStatuses.size(); }
    int activeRoleCount() const { return m_activeRoleCount; }
    int idleRoleCount() const { return m_idleRoleCount; }
    int blockedRoleCount() const { return m_blockedRoleCount; }
    int offlineRoleCount() const { return m_offlineRoleCount; }

    // Pipeline accessors
    QVariantList pipelineItems() const { return m_pipelineItems; }
    int activeTaskCount() const { return m_activeTaskCount; }
    int blockedTaskCount() const { return m_blockedTaskCount; }
    int completedTaskCount() const { return m_completedTaskCount; }

    // Monitoring accessors
    bool monitoring() const;
    QString lastUpdateTime() const { return m_lastUpdateTime; }
    int refreshCount() const { return m_refreshCount; }
    QString lastError() const { return m_lastError; }

    // QML invokables
    Q_INVOKABLE void startMonitoring();
    Q_INVOKABLE void stopMonitoring();
    Q_INVOKABLE void refreshNow();
    Q_INVOKABLE QVariantMap roleStatus(const QString& roleId) const;
    Q_INVOKABLE int completedToday(const QString& roleId) const;
    Q_INVOKABLE QString roleStateText(const QString& roleId) const;

    /**
     * @brief Get a formatted summary string for display
     */
    Q_INVOKABLE QString teamSummary() const;

signals:
    void roleStatusesChanged();
    void roleStatusCardsChanged();
    void pipelineItemsChanged();
    void monitoringChanged();
    void lastUpdateTimeChanged();
    void refreshCountChanged();
    void lastErrorChanged();

private slots:
    void onRoleStatusesUpdated(const QVector<TeamRoleStatus>& statuses);
    void onPipelineItemsUpdated(const QVector<TaskPipelineItem>& items);
    void onDataUpdated();
    void onRefreshFailed(const QString& error);
    void onMonitoringStateChanged(bool active);

private:
    TeamMonitorService* m_monitorService;

    // Cached QML-friendly data
    QVariantList m_roleStatuses;
    QVariantList m_roleStatusCards;
    int m_activeRoleCount = 0;
    int m_idleRoleCount = 0;
    int m_blockedRoleCount = 0;
    int m_offlineRoleCount = 0;

    QVariantList m_pipelineItems;
    int m_activeTaskCount = 0;
    int m_blockedTaskCount = 0;
    int m_completedTaskCount = 0;

    QString m_lastUpdateTime;
    int m_refreshCount = 0;
    QString m_lastError;

    void updateCounts();
    void buildRoleStatusCards();
    static QString formatDuration(qint64 elapsedMs);
};

} // namespace TeamOps

#endif // TEAMOPSVIEWMODEL_H
