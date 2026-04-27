#ifndef FILETEAMDATAPROVIDER_H
#define FILETEAMDATAPROVIDER_H

#include "ITeamDataProvider.h"
#include <QFileSystemWatcher>

namespace TeamOps {

/**
 * @brief Team data provider that reads from a JSON file
 *
 * Watches a JSON file for changes (via QFileSystemWatcher)
 * and parses team status data. This allows external processes
 * (e.g., AI agent scripts) to inject real-time status by
 * writing to the monitored file.
 *
 * Expected JSON format:
 * @code
 * {
 *   "roles": [
 *     {
 *       "roleId": "domain-dev",
 *       "displayName": "领域引擎开发",
 *       "state": "executing",
 *       "currentTaskId": "task-42a1",
 *       "currentTaskSummary": "...",
 *       "completedToday": 2,
 *       "totalCompleted": 15
 *     }
 *   ],
 *   "pipeline": [
 *     {
 *       "taskId": "task-42a1",
 *       "title": "...",
 *       "currentStage": "development",
 *       "ownerRole": "domain-dev",
 *       "isBlocked": false
 *     }
 *   ]
 * }
 * @endcode
 */
class FileTeamDataProvider : public ITeamDataProvider {
    Q_OBJECT

public:
    explicit FileTeamDataProvider(const QString& filePath, QObject* parent = nullptr);
    ~FileTeamDataProvider() override = default;

    QVector<TeamRoleStatus> fetchRoleStatuses() override;
    QVector<TaskPipelineItem> fetchPipelineItems() override;
    bool isAvailable() const override;
    QString providerName() const override { return QStringLiteral("FileProvider:"); }

    /**
     * @brief Change the monitored file path
     */
    void setFilePath(const QString& filePath);

    QString filePath() const { return m_filePath; }

private slots:
    void onFileChanged();

private:
    QString m_filePath;
    QFileSystemWatcher* m_watcher;
    QVector<TeamRoleStatus> m_cachedRoleStatuses;
    QVector<TaskPipelineItem> m_cachedPipelineItems;
    bool m_dataValid = false;

    bool parseJsonFile();
    TeamRoleStatus parseRoleStatus(const QJsonObject& obj);
    TaskPipelineItem parsePipelineItem(const QJsonObject& obj);
};

} // namespace TeamOps

#endif // FILETEAMDATAPROVIDER_H
