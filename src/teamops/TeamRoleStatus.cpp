#include "TeamRoleStatus.h"

namespace TeamOps {

QVariantMap TeamRoleStatus::toVariantMap() const
{
    QVariantMap map;
    map[QStringLiteral("roleId")] = roleId;
    map[QStringLiteral("displayName")] = displayName;
    map[QStringLiteral("state")] = teamRoleStateToString(state);
    map[QStringLiteral("stateEnum")] = static_cast<int>(state);
    map[QStringLiteral("currentTaskId")] = currentTaskId;
    map[QStringLiteral("currentTaskSummary")] = currentTaskSummary;
    map[QStringLiteral("taskStartTime")] = taskStartTime.toString(Qt::ISODate);
    map[QStringLiteral("completedToday")] = completedToday;
    map[QStringLiteral("totalCompleted")] = totalCompleted;
    map[QStringLiteral("blockedCount")] = blockedCount;
    map[QStringLiteral("lastError")] = lastError;
    map[QStringLiteral("lastSeen")] = lastSeen.toString(Qt::ISODate);
    return map;
}

} // namespace TeamOps
