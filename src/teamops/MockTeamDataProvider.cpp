#include "MockTeamDataProvider.h"
#include <QDateTime>

namespace TeamOps {

MockTeamDataProvider::MockTeamDataProvider(QObject* parent)
    : ITeamDataProvider(parent)
{
    initializeMockData();
}

QVector<TeamRoleStatus> MockTeamDataProvider::fetchRoleStatuses()
{
    // Update lastSeen timestamps
    const QDateTime now = QDateTime::currentDateTime();
    for (auto& status : m_roleStatuses) {
        status.lastSeen = now;
    }
    return m_roleStatuses;
}

QVector<TaskPipelineItem> MockTeamDataProvider::fetchPipelineItems()
{
    updatePipelineFromRoles();
    return m_pipelineItems;
}

void MockTeamDataProvider::setRoleState(const QString& roleId, TeamRoleState state)
{
    for (auto& status : m_roleStatuses) {
        if (status.roleId == roleId) {
            status.state = state;
            status.lastSeen = QDateTime::currentDateTime();
            if (state == TeamRoleState::Executing && status.currentTaskId.isEmpty()) {
                status.currentTaskId = QStringLiteral("mock-task-") + roleId;
                status.currentTaskSummary = QStringLiteral("Simulated task for ") + roleId;
                status.taskStartTime = QDateTime::currentDateTime();
            }
            break;
        }
    }
}

void MockTeamDataProvider::simulateTaskCompletion(const QString& roleId)
{
    for (auto& status : m_roleStatuses) {
        if (status.roleId == roleId) {
            status.state = TeamRoleState::Idle;
            status.completedToday++;
            status.totalCompleted++;
            status.currentTaskId.clear();
            status.currentTaskSummary.clear();
            break;
        }
    }
}

void MockTeamDataProvider::advanceSimulation()
{
    m_simulationTick++;
    const int tick = m_simulationTick;

    // Rotate some role states to simulate activity
    static const QVector<QString> roleIds = {
        QStringLiteral("domain-dev"),
        QStringLiteral("protocol-dev"),
        QStringLiteral("viewmodel-dev"),
        QStringLiteral("ui-dev"),
        QStringLiteral("infra-dev"),
        QStringLiteral("reviewer")
    };

    for (int i = 0; i < roleIds.size(); ++i) {
        // Each role cycles through states on different periods
        const int period = 3 + (i % 4);
        const int phase = (tick / period) % 5;

        TeamRoleState newState;
        switch (phase) {
        case 0: newState = TeamRoleState::Idle; break;
        case 1: newState = TeamRoleState::Executing; break;
        case 2: newState = TeamRoleState::Executing; break;
        case 3: newState = TeamRoleState::ReviewPending; break;
        case 4: newState = TeamRoleState::Idle; break;
        default: newState = TeamRoleState::Idle; break;
        }

        setRoleState(roleIds[i], newState);
    }

    // Occasionally set a role as blocked
    if (tick % 7 == 0) {
        setRoleState(roleIds[tick % roleIds.size()], TeamRoleState::Blocked);
    }

    emit dataChanged();
}

void MockTeamDataProvider::initializeMockData()
{
    const QDateTime now = QDateTime::currentDateTime();

    struct RoleInit {
        QString id;
        QString name;
        TeamRoleState state;
        QString taskId;
        QString taskSummary;
        int completed;
    };

    const QVector<RoleInit> roles = {
        { QStringLiteral("leader"), QStringLiteral("团队负责人"), TeamRoleState::Executing,
          QStringLiteral("task-277d"), QStringLiteral("分配任务与协调进度"), 12 },
        { QStringLiteral("domain-dev"), QStringLiteral("领域引擎开发"), TeamRoleState::Executing,
          QStringLiteral("task-42a1"), QStringLiteral("GearboxTestEngine状态机优化"), 8 },
        { QStringLiteral("protocol-dev"), QStringLiteral("协议与设备开发"), TeamRoleState::Idle,
          QString(), QString(), 15 },
        { QStringLiteral("viewmodel-dev"), QStringLiteral("ViewModel 开发"), TeamRoleState::Blocked,
          QStringLiteral("task-9e3f"), QStringLiteral("等待Domain层接口确认"), 6 },
        { QStringLiteral("ui-dev"), QStringLiteral("UI/QML 开发"), TeamRoleState::Executing,
          QStringLiteral("task-b88c"), QStringLiteral("TestExecutionPage StateMachine重构"), 10 },
        { QStringLiteral("infra-dev"), QStringLiteral("基础设施/服务开发"), TeamRoleState::ReviewPending,
          QStringLiteral("task-d701"), QStringLiteral("TeamMonitorService实现"), 9 },
        { QStringLiteral("reviewer"), QStringLiteral("代码审查"), TeamRoleState::Executing,
          QStringLiteral("task-f2a0"), QStringLiteral("审查Modbus协议层代码"), 20 }
    };

    for (const auto& r : roles) {
        TeamRoleStatus status;
        status.roleId = r.id;
        status.displayName = r.name;
        status.state = r.state;
        status.currentTaskId = r.taskId;
        status.currentTaskSummary = r.taskSummary;
        status.totalCompleted = r.completed;
        status.completedToday = (r.state == TeamRoleState::Idle) ? 1 : 0;
        status.taskStartTime = now;
        status.lastSeen = now;
        m_roleStatuses.append(status);
    }

    updatePipelineFromRoles();
}

void MockTeamDataProvider::updatePipelineFromRoles()
{
    m_pipelineItems.clear();

    struct PipelineInit {
        QString taskId;
        QString title;
        PipelineStage stage;
        QString owner;
        bool blocked;
        QString blockReason;
    };

    const QVector<PipelineInit> items = {
        { QStringLiteral("task-277d"), QStringLiteral("项目架构分析与迭代方案设计"),
          PipelineStage::Deployment, QStringLiteral("leader"), false, QString() },
        { QStringLiteral("task-42a1"), QStringLiteral("GearboxTestEngine Phase拆分重构"),
          PipelineStage::Development, QStringLiteral("domain-dev"), false, QString() },
        { QStringLiteral("task-b88c"), QStringLiteral("TestExecutionPage QML StateMachine"),
          PipelineStage::Development, QStringLiteral("ui-dev"), false, QString() },
        { QStringLiteral("task-d701"), QStringLiteral("TeamOps监控服务基础实现"),
          PipelineStage::CodeReview, QStringLiteral("infra-dev"), false, QString() },
        { QStringLiteral("task-9e3f"), QStringLiteral("ViewModel绑定优化"),
          PipelineStage::Development, QStringLiteral("viewmodel-dev"), true,
          QStringLiteral("等待Domain层接口定义") },
        { QStringLiteral("task-5c21"), QStringLiteral("Mock设备故障注入扩展"),
          PipelineStage::Test, QStringLiteral("protocol-dev"), false, QString() },
        { QStringLiteral("task-aef3"), QStringLiteral("QML交互测试框架"),
          PipelineStage::Development, QStringLiteral("ui-dev"), false, QString() }
    };

    const QDateTime now = QDateTime::currentDateTime();
    for (const auto& p : items) {
        TaskPipelineItem item;
        item.taskId = p.taskId;
        item.title = p.title;
        item.currentStage = p.stage;
        item.ownerRole = p.owner;
        item.stageStartTime = now.addSecs(-3600 * (static_cast<int>(p.stage) + 1));
        item.estimatedDurationMs = 4 * 3600 * 1000; // 4 hours
        item.isBlocked = p.blocked;
        item.blockReason = p.blockReason;
        m_pipelineItems.append(item);
    }
}

} // namespace TeamOps
