#include "ITeamDataProvider.h"

namespace TeamOps {

QVariantMap TaskPipelineItem::toVariantMap() const
{
    QVariantMap map;
    map[QStringLiteral("taskId")] = taskId;
    map[QStringLiteral("title")] = title;
    map[QStringLiteral("currentStage")] = pipelineStageToString(currentStage);
    map[QStringLiteral("stageEnum")] = static_cast<int>(currentStage);
    map[QStringLiteral("ownerRole")] = ownerRole;
    map[QStringLiteral("stageStartTime")] = stageStartTime.toString(Qt::ISODate);
    map[QStringLiteral("estimatedDurationMs")] = static_cast<qint64>(estimatedDurationMs);
    map[QStringLiteral("isBlocked")] = isBlocked;
    map[QStringLiteral("blockReason")] = blockReason;
    return map;
}

} // namespace TeamOps
