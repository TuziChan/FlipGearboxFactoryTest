#ifndef ITEAMDATAPROVIDER_H
#define ITEAMDATAPROVIDER_H

#include <QObject>
#include <QVector>
#include <QJsonObject>
#include "TeamRoleStatus.h"

namespace TeamOps {

/**
 * @brief Pipeline stage for a task workflow
 */
enum class PipelineStage {
    Development,
    CodeReview,
    Build,
    Test,
    Deployment,
    Done
};

inline QString pipelineStageToString(PipelineStage stage)
{
    switch (stage) {
    case PipelineStage::Development: return QStringLiteral("development");
    case PipelineStage::CodeReview: return QStringLiteral("code_review");
    case PipelineStage::Build: return QStringLiteral("build");
    case PipelineStage::Test: return QStringLiteral("test");
    case PipelineStage::Deployment: return QStringLiteral("deployment");
    case PipelineStage::Done: return QStringLiteral("done");
    }
    return QStringLiteral("unknown");
}

/**
 * @brief Represents a single item in the task pipeline
 */
struct TaskPipelineItem {
    QString taskId;
    QString title;
    PipelineStage currentStage;
    QString ownerRole;
    QDateTime stageStartTime;
    qint64 estimatedDurationMs;
    bool isBlocked;
    QString blockReason;

    TaskPipelineItem()
        : currentStage(PipelineStage::Development)
        , estimatedDurationMs(0)
        , isBlocked(false)
    {}

    QVariantMap toVariantMap() const;
};

/**
 * @brief Abstract interface for team data sources
 *
 * Implementations can fetch data from MCP tools, files,
 * network APIs, or mock data for testing.
 */
class ITeamDataProvider : public QObject {
    Q_OBJECT

public:
    explicit ITeamDataProvider(QObject* parent = nullptr) : QObject(parent) {}
    ~ITeamDataProvider() override = default;

    /**
     * @brief Fetch current status for all team roles
     * @return Vector of role statuses
     */
    virtual QVector<TeamRoleStatus> fetchRoleStatuses() = 0;

    /**
     * @brief Fetch current task pipeline items
     * @return Vector of pipeline items
     */
    virtual QVector<TaskPipelineItem> fetchPipelineItems() = 0;

    /**
     * @brief Check if provider is available and ready
     */
    virtual bool isAvailable() const = 0;

    /**
     * @brief Human-readable provider name
     */
    virtual QString providerName() const = 0;

    /**
     * @brief Last error message (if any)
     */
    QString lastError() const { return m_lastError; }

signals:
    /**
     * @brief Emitted when provider detects data changes
     */
    void dataChanged();

protected:
    void setLastError(const QString& error) { m_lastError = error; }
    void clearError() { m_lastError.clear(); }

    QString m_lastError;
};

} // namespace TeamOps

Q_DECLARE_METATYPE(TeamOps::TaskPipelineItem)

#endif // ITEAMDATAPROVIDER_H
