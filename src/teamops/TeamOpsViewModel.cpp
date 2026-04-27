#include "TeamOpsViewModel.h"
#include <QDebug>

namespace TeamOps {

TeamOpsViewModel::TeamOpsViewModel(TeamMonitorService* monitorService,
                                     QObject* parent)
    : QObject(parent)
    , m_monitorService(monitorService)
{
    Q_ASSERT(monitorService != nullptr);

    connect(monitorService, &TeamMonitorService::roleStatusesUpdated,
            this, &TeamOpsViewModel::onRoleStatusesUpdated);
    connect(monitorService, &TeamMonitorService::pipelineItemsUpdated,
            this, &TeamOpsViewModel::onPipelineItemsUpdated);
    connect(monitorService, &TeamMonitorService::dataUpdated,
            this, &TeamOpsViewModel::onDataUpdated);
    connect(monitorService, &TeamMonitorService::refreshFailed,
            this, &TeamOpsViewModel::onRefreshFailed);
    connect(monitorService, &TeamMonitorService::monitoringStateChanged,
            this, &TeamOpsViewModel::onMonitoringStateChanged);

    // Initial data sync if service already has data
    onDataUpdated();
}

bool TeamOpsViewModel::monitoring() const
{
    return m_monitorService ? m_monitorService->isMonitoring() : false;
}

void TeamOpsViewModel::startMonitoring()
{
    if (m_monitorService) {
        m_monitorService->startMonitoring();
    }
}

void TeamOpsViewModel::stopMonitoring()
{
    if (m_monitorService) {
        m_monitorService->stopMonitoring();
    }
}

void TeamOpsViewModel::refreshNow()
{
    if (m_monitorService) {
        m_monitorService->refreshNow();
    }
}

QVariantMap TeamOpsViewModel::roleStatus(const QString& roleId) const
{
    for (const auto& item : m_roleStatuses) {
        const QVariantMap map = item.toMap();
        if (map.value(QStringLiteral("roleId")).toString() == roleId) {
            return map;
        }
    }
    return QVariantMap();
}

int TeamOpsViewModel::completedToday(const QString& roleId) const
{
    const QVariantMap status = roleStatus(roleId);
    return status.value(QStringLiteral("completedToday"), 0).toInt();
}

QString TeamOpsViewModel::roleStateText(const QString& roleId) const
{
    const QVariantMap status = roleStatus(roleId);
    const QString state = status.value(QStringLiteral("state")).toString();

    static const QMap<QString, QString> stateMap = {
        { QStringLiteral("idle"), QStringLiteral("空闲") },
        { QStringLiteral("executing"), QStringLiteral("执行中") },
        { QStringLiteral("blocked"), QStringLiteral("阻塞") },
        { QStringLiteral("review_pending"), QStringLiteral("待审核") },
        { QStringLiteral("offline"), QStringLiteral("离线") }
    };

    return stateMap.value(state, state);
}

QString TeamOpsViewModel::teamSummary() const
{
    return QStringLiteral("团队共%1人，活跃%2人，空闲%3人，阻塞%4人，离线%5人 | 流水线任务%6个，阻塞%7个")
        .arg(totalRoleCount())
        .arg(activeRoleCount())
        .arg(idleRoleCount())
        .arg(blockedRoleCount())
        .arg(offlineRoleCount())
        .arg(activeTaskCount())
        .arg(blockedTaskCount());
}

void TeamOpsViewModel::onRoleStatusesUpdated(const QVector<TeamRoleStatus>& statuses)
{
    QVariantList list;
    for (const auto& status : statuses) {
        list.append(status.toVariantMap());
    }

    if (m_roleStatuses != list) {
        m_roleStatuses = list;
        updateCounts();
        buildRoleStatusCards();
        emit roleStatusesChanged();
        emit roleStatusCardsChanged();
    }
}

void TeamOpsViewModel::onPipelineItemsUpdated(const QVector<TaskPipelineItem>& items)
{
    QVariantList list;
    int completed = 0;
    int blocked = 0;

    for (const auto& item : items) {
        QVariantMap map = item.toVariantMap();

        // Add derived display fields
        if (item.isBlocked) {
            map[QStringLiteral("statusText")] = QStringLiteral("阻塞: ") + item.blockReason;
            map[QStringLiteral("statusColor")] = QStringLiteral("#EF4444"); // red
            blocked++;
        } else if (item.currentStage == PipelineStage::Done) {
            map[QStringLiteral("statusText")] = QStringLiteral("已完成");
            map[QStringLiteral("statusColor")] = QStringLiteral("#22C55E"); // green
            completed++;
        } else {
            const QString stageText = [&]() -> QString {
                switch (item.currentStage) {
                case PipelineStage::Development: return QStringLiteral("开发中");
                case PipelineStage::CodeReview: return QStringLiteral("代码审查");
                case PipelineStage::Build: return QStringLiteral("构建中");
                case PipelineStage::Test: return QStringLiteral("测试中");
                case PipelineStage::Deployment: return QStringLiteral("部署中");
                default: return QStringLiteral("未知");
                }
            }();
            map[QStringLiteral("statusText")] = stageText;
            map[QStringLiteral("statusColor")] = QStringLiteral("#3B82F6"); // blue
        }

        // Calculate elapsed time
        const qint64 elapsedMs = item.stageStartTime.msecsTo(QDateTime::currentDateTime());
        map[QStringLiteral("elapsedText")] = formatDuration(elapsedMs);

        list.append(map);
    }

    m_pipelineItems = list;
    m_activeTaskCount = static_cast<int>(items.size()) - completed;
    m_blockedTaskCount = blocked;
    m_completedTaskCount = completed;

    emit pipelineItemsChanged();
}

void TeamOpsViewModel::onDataUpdated()
{
    m_refreshCount = m_monitorService ? m_monitorService->refreshCount() : 0;
    m_lastUpdateTime = QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss"));

    emit refreshCountChanged();
    emit lastUpdateTimeChanged();
}

void TeamOpsViewModel::onRefreshFailed(const QString& error)
{
    m_lastError = error;
    emit lastErrorChanged();
    qWarning() << "[TeamOpsViewModel] Refresh failed:" << error;
}

void TeamOpsViewModel::onMonitoringStateChanged(bool active)
{
    emit monitoringChanged();
    qDebug() << "[TeamOpsViewModel] Monitoring state:" << active;
}

void TeamOpsViewModel::updateCounts()
{
    int active = 0, idle = 0, blocked = 0, offline = 0;

    for (const auto& item : m_roleStatuses) {
        const QVariantMap map = item.toMap();
        const QString state = map.value(QStringLiteral("state")).toString();

        if (state == QStringLiteral("executing") || state == QStringLiteral("review_pending")) {
            active++;
        } else if (state == QStringLiteral("idle")) {
            idle++;
        } else if (state == QStringLiteral("blocked")) {
            blocked++;
        } else if (state == QStringLiteral("offline")) {
            offline++;
        }
    }

    m_activeRoleCount = active;
    m_idleRoleCount = idle;
    m_blockedRoleCount = blocked;
    m_offlineRoleCount = offline;
}

void TeamOpsViewModel::buildRoleStatusCards()
{
    QVariantList cards;

    for (const auto& item : m_roleStatuses) {
        QVariantMap map = item.toMap();
        const QString state = map.value(QStringLiteral("state")).toString();

        // Add display-friendly fields
        static const QMap<QString, QString> stateColorMap = {
            { QStringLiteral("idle"), QStringLiteral("#22C55E") },          // green
            { QStringLiteral("executing"), QStringLiteral("#3B82F6") },     // blue
            { QStringLiteral("blocked"), QStringLiteral("#EF4444") },       // red
            { QStringLiteral("review_pending"), QStringLiteral("#F59E0B") }, // amber
            { QStringLiteral("offline"), QStringLiteral("#6B7280") }        // gray
        };

        static const QMap<QString, QString> stateTextMap = {
            { QStringLiteral("idle"), QStringLiteral("空闲") },
            { QStringLiteral("executing"), QStringLiteral("执行中") },
            { QStringLiteral("blocked"), QStringLiteral("阻塞") },
            { QStringLiteral("review_pending"), QStringLiteral("待审核") },
            { QStringLiteral("offline"), QStringLiteral("离线") }
        };

        map[QStringLiteral("stateColor")] = stateColorMap.value(state, QStringLiteral("#6B7280"));
        map[QStringLiteral("stateText")] = stateTextMap.value(state, state);

        // Calculate task duration if executing
        if (state == QStringLiteral("executing")) {
            const QString startStr = map.value(QStringLiteral("taskStartTime")).toString();
            const QDateTime startTime = QDateTime::fromString(startStr, Qt::ISODate);
            if (startTime.isValid()) {
                const qint64 elapsedMs = startTime.msecsTo(QDateTime::currentDateTime());
                map[QStringLiteral("taskDuration")] = formatDuration(elapsedMs);
            }
        }

        // Progress indicator (mock based on state)
        if (state == QStringLiteral("executing")) {
            map[QStringLiteral("progressPercent")] = 65; // Mock progress
        } else if (state == QStringLiteral("review_pending")) {
            map[QStringLiteral("progressPercent")] = 90;
        } else if (state == QStringLiteral("idle")) {
            map[QStringLiteral("progressPercent")] = 0;
        } else {
            map[QStringLiteral("progressPercent")] = 0;
        }

        cards.append(map);
    }

    m_roleStatusCards = cards;
}

QString TeamOpsViewModel::formatDuration(qint64 elapsedMs)
{
    if (elapsedMs < 0) elapsedMs = 0;

    const int seconds = static_cast<int>((elapsedMs / 1000) % 60);
    const int minutes = static_cast<int>((elapsedMs / (1000 * 60)) % 60);
    const int hours = static_cast<int>(elapsedMs / (1000 * 60 * 60));

    if (hours > 0) {
        return QStringLiteral("%1小时%2分").arg(hours).arg(minutes);
    } else if (minutes > 0) {
        return QStringLiteral("%1分%2秒").arg(minutes).arg(seconds);
    } else {
        return QStringLiteral("%1秒").arg(seconds);
    }
}

} // namespace TeamOps
