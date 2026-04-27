#include "FileTeamDataProvider.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

namespace TeamOps {

FileTeamDataProvider::FileTeamDataProvider(const QString& filePath, QObject* parent)
    : ITeamDataProvider(parent)
    , m_filePath(filePath)
    , m_watcher(new QFileSystemWatcher(this))
{
    connect(m_watcher, &QFileSystemWatcher::fileChanged,
            this, &FileTeamDataProvider::onFileChanged);

    if (!m_filePath.isEmpty()) {
        setFilePath(m_filePath);
    }
}

void FileTeamDataProvider::setFilePath(const QString& filePath)
{
    if (!m_filePath.isEmpty()) {
        m_watcher->removePath(m_filePath);
    }

    m_filePath = filePath;

    if (!m_filePath.isEmpty() && QFile::exists(m_filePath)) {
        m_watcher->addPath(m_filePath);
        parseJsonFile();
    }
}

QVector<TeamRoleStatus> FileTeamDataProvider::fetchRoleStatuses()
{
    if (!m_dataValid) {
        parseJsonFile();
    }
    return m_cachedRoleStatuses;
}

QVector<TaskPipelineItem> FileTeamDataProvider::fetchPipelineItems()
{
    if (!m_dataValid) {
        parseJsonFile();
    }
    return m_cachedPipelineItems;
}

bool FileTeamDataProvider::isAvailable() const
{
    return !m_filePath.isEmpty() && QFile::exists(m_filePath);
}

void FileTeamDataProvider::onFileChanged()
{
    if (parseJsonFile()) {
        emit dataChanged();
    }
}

bool FileTeamDataProvider::parseJsonFile()
{
    if (m_filePath.isEmpty() || !QFile::exists(m_filePath)) {
        setLastError(QStringLiteral("File does not exist: ") + m_filePath);
        m_dataValid = false;
        return false;
    }

    QFile file(m_filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        setLastError(QStringLiteral("Cannot open file: ") + file.errorString());
        m_dataValid = false;
        return false;
    }

    const QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        setLastError(QStringLiteral("JSON parse error: ") + parseError.errorString());
        m_dataValid = false;
        return false;
    }

    const QJsonObject root = doc.object();

    // Parse roles
    m_cachedRoleStatuses.clear();
    const QJsonArray rolesArray = root.value(QStringLiteral("roles")).toArray();
    for (const auto& value : rolesArray) {
        m_cachedRoleStatuses.append(parseRoleStatus(value.toObject()));
    }

    // Parse pipeline
    m_cachedPipelineItems.clear();
    const QJsonArray pipelineArray = root.value(QStringLiteral("pipeline")).toArray();
    for (const auto& value : pipelineArray) {
        m_cachedPipelineItems.append(parsePipelineItem(value.toObject()));
    }

    clearError();
    m_dataValid = true;
    return true;
}

TeamRoleStatus FileTeamDataProvider::parseRoleStatus(const QJsonObject& obj)
{
    TeamRoleStatus status;
    status.roleId = obj.value(QStringLiteral("roleId")).toString();
    status.displayName = obj.value(QStringLiteral("displayName")).toString();
    status.state = teamRoleStateFromString(obj.value(QStringLiteral("state")).toString());
    status.currentTaskId = obj.value(QStringLiteral("currentTaskId")).toString();
    status.currentTaskSummary = obj.value(QStringLiteral("currentTaskSummary")).toString();
    status.completedToday = obj.value(QStringLiteral("completedToday")).toInt();
    status.totalCompleted = obj.value(QStringLiteral("totalCompleted")).toInt();
    status.blockedCount = obj.value(QStringLiteral("blockedCount")).toInt();
    status.lastError = obj.value(QStringLiteral("lastError")).toString();

    const QString taskStartStr = obj.value(QStringLiteral("taskStartTime")).toString();
    if (!taskStartStr.isEmpty()) {
        status.taskStartTime = QDateTime::fromString(taskStartStr, Qt::ISODate);
    }

    const QString lastSeenStr = obj.value(QStringLiteral("lastSeen")).toString();
    if (!lastSeenStr.isEmpty()) {
        status.lastSeen = QDateTime::fromString(lastSeenStr, Qt::ISODate);
    } else {
        status.lastSeen = QDateTime::currentDateTime();
    }

    return status;
}

TaskPipelineItem FileTeamDataProvider::parsePipelineItem(const QJsonObject& obj)
{
    TaskPipelineItem item;
    item.taskId = obj.value(QStringLiteral("taskId")).toString();
    item.title = obj.value(QStringLiteral("title")).toString();
    item.ownerRole = obj.value(QStringLiteral("ownerRole")).toString();
    item.isBlocked = obj.value(QStringLiteral("isBlocked")).toBool();
    item.blockReason = obj.value(QStringLiteral("blockReason")).toString();
    item.estimatedDurationMs = obj.value(QStringLiteral("estimatedDurationMs")).toVariant().toLongLong();

    const QString stageStr = obj.value(QStringLiteral("currentStage")).toString().toLower();
    if (stageStr == QStringLiteral("development")) item.currentStage = PipelineStage::Development;
    else if (stageStr == QStringLiteral("code_review") || stageStr == QStringLiteral("codereview"))
        item.currentStage = PipelineStage::CodeReview;
    else if (stageStr == QStringLiteral("build")) item.currentStage = PipelineStage::Build;
    else if (stageStr == QStringLiteral("test")) item.currentStage = PipelineStage::Test;
    else if (stageStr == QStringLiteral("deployment")) item.currentStage = PipelineStage::Deployment;
    else if (stageStr == QStringLiteral("done")) item.currentStage = PipelineStage::Done;

    const QString stageStartStr = obj.value(QStringLiteral("stageStartTime")).toString();
    if (!stageStartStr.isEmpty()) {
        item.stageStartTime = QDateTime::fromString(stageStartStr, Qt::ISODate);
    }

    return item;
}

} // namespace TeamOps
