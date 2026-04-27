#ifndef MOCKTEAMDATAPROVIDER_H
#define MOCKTEAMDATAPROVIDER_H

#include "ITeamDataProvider.h"

namespace TeamOps {

/**
 * @brief Mock data provider for demonstration and testing
 *
 * Generates simulated team role statuses and pipeline items
 * without requiring external MCP connections.
 */
class MockTeamDataProvider : public ITeamDataProvider {
    Q_OBJECT

public:
    explicit MockTeamDataProvider(QObject* parent = nullptr);
    ~MockTeamDataProvider() override = default;

    QVector<TeamRoleStatus> fetchRoleStatuses() override;
    QVector<TaskPipelineItem> fetchPipelineItems() override;
    bool isAvailable() const override { return true; }
    QString providerName() const override { return QStringLiteral("MockProvider"); }

    /**
     * @brief Set a specific role state for testing
     */
    void setRoleState(const QString& roleId, TeamRoleState state);

    /**
     * @brief Simulate task completion for a role
     */
    void simulateTaskCompletion(const QString& roleId);

    /**
     * @brief Advance simulation (rotate states automatically)
     */
    void advanceSimulation();

private:
    QVector<TeamRoleStatus> m_roleStatuses;
    QVector<TaskPipelineItem> m_pipelineItems;
    int m_simulationTick = 0;

    void initializeMockData();
    void updatePipelineFromRoles();
};

} // namespace TeamOps

#endif // MOCKTEAMDATAPROVIDER_H
