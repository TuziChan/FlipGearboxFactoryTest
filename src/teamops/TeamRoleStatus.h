#ifndef TEAMROLESTATUS_H
#define TEAMROLESTATUS_H

#include <QObject>
#include <QString>
#include <QMetaType>
#include <QDateTime>

namespace TeamOps {

/**
 * @brief Enumeration of possible states for a team role
 */
enum class TeamRoleState {
    Idle,           /**< Role is idle, no task assigned */
    Executing,      /**< Role is actively executing a task */
    Blocked,        /**< Role is blocked waiting for dependencies */
    ReviewPending,  /**< Role has completed work pending review */
    Offline         /**< Role is offline or disconnected */
};

/**
 * @brief Convert TeamRoleState to human-readable string
 */
inline QString teamRoleStateToString(TeamRoleState state)
{
    switch (state) {
    case TeamRoleState::Idle: return QStringLiteral("idle");
    case TeamRoleState::Executing: return QStringLiteral("executing");
    case TeamRoleState::Blocked: return QStringLiteral("blocked");
    case TeamRoleState::ReviewPending: return QStringLiteral("review_pending");
    case TeamRoleState::Offline: return QStringLiteral("offline");
    }
    return QStringLiteral("unknown");
}

/**
 * @brief Convert string to TeamRoleState
 */
inline TeamRoleState teamRoleStateFromString(const QString& str)
{
    const QString lower = str.toLower();
    if (lower == QStringLiteral("idle")) return TeamRoleState::Idle;
    if (lower == QStringLiteral("executing")) return TeamRoleState::Executing;
    if (lower == QStringLiteral("blocked")) return TeamRoleState::Blocked;
    if (lower == QStringLiteral("review_pending") || lower == QStringLiteral("reviewpending")) return TeamRoleState::ReviewPending;
    if (lower == QStringLiteral("offline")) return TeamRoleState::Offline;
    return TeamRoleState::Offline;
}

/**
 * @brief Status information for a single team role
 *
 * Represents the real-time state of an AI team member,
 * including current task, progress, and historical metrics.
 */
struct TeamRoleStatus {
    QString roleId;              /**< Role identifier (e.g., "domain-dev") */
    QString displayName;         /**< Human-readable role name */
    TeamRoleState state;         /**< Current state */
    QString currentTaskId;       /**< ID of currently assigned task */
    QString currentTaskSummary;  /**< Brief description of current task */
    QDateTime taskStartTime;     /**< When the current task started */
    int completedToday;          /**< Number of tasks completed today */
    int totalCompleted;          /**< Total tasks completed (all time) */
    int blockedCount;            /**< Times blocked in current session */
    QString lastError;           /**< Last error message (if any) */
    QDateTime lastSeen;          /**< Last time this role reported status */

    TeamRoleStatus()
        : state(TeamRoleState::Offline)
        , completedToday(0)
        , totalCompleted(0)
        , blockedCount(0)
    {}

    /**
     * @brief Convert to QVariantMap for QML binding
     */
    QVariantMap toVariantMap() const;
};

} // namespace TeamOps

Q_DECLARE_METATYPE(TeamOps::TeamRoleState)
Q_DECLARE_METATYPE(TeamOps::TeamRoleStatus)

#endif // TEAMROLESTATUS_H
