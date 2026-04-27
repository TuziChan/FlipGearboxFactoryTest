#ifndef MOCKTESTRUNNER_H
#define MOCKTESTRUNNER_H

#include "AutoTestFramework.h"
#include "../../src/infrastructure/simulation/MockDeviceManager.h"
#include "../../src/infrastructure/simulation/SimulationContext.h"
#include <QObject>
#include <memory>

namespace Tests {
namespace Framework {

/**
 * @brief Test scenario configuration for mock-based testing
 */
struct MockTestScenario {
    QString name;
    QString description;
    bool enableErrorInjection = false;
    double crcErrorRate = 0.0;
    double timeoutRate = 0.0;
    double delayRate = 0.0;
    bool simulateHighSpeed = false;
    int highSpeedDurationMs = 1000;
    int highSpeedRateHz = 100;
};

/**
 * @brief Comprehensive mock test runner for hardware simulation testing
 *
 * Provides:
 * - Automatic mock device lifecycle management
 * - Predefined test scenarios (normal, error, stress)
 * - Integration with StationRuntime for end-to-end testing
 * - Performance metrics collection during mock execution
 */
class MockTestRunner : public AutoTestBase {
    Q_OBJECT

public:
    explicit MockTestRunner(QObject* parent = nullptr);
    ~MockTestRunner() override;

    /**
     * @brief Initialize the mock environment with default devices
     */
    bool initializeMockEnvironment();

    /**
     * @brief Shutdown and cleanup all mock resources
     */
    void shutdownMockEnvironment();

    /**
     * @brief Get the mock device manager
     */
    Infrastructure::Simulation::MockDeviceManager* mockDeviceManager() const { return m_mockManager.get(); }

    /**
     * @brief Get the simulation context
     */
    std::shared_ptr<Infrastructure::Simulation::SimulationContext> simulationContext() const { return m_simContext; }

    /**
     * @brief Run a predefined test scenario
     */
    bool runScenario(const MockTestScenario& scenario);

    /**
     * @brief Get available predefined scenarios
     */
    static QVector<MockTestScenario> predefinedScenarios();

    // Predefined scenario executors
    bool runNormalOperationScenario();
    bool runErrorInjectionScenario();
    bool runStressTestScenario();
    bool runAngleTestWithMagnetDetection();
    bool runMotorLoadSimulationScenario();

signals:
    void scenarioStarted(const QString& scenarioName);
    void scenarioFinished(const QString& scenarioName, bool passed, qint64 durationMs);
    void deviceStateChanged(const QString& deviceName, const QString& state);

private:
    std::unique_ptr<Infrastructure::Simulation::MockDeviceManager> m_mockManager;
    std::shared_ptr<Infrastructure::Simulation::SimulationContext> m_simContext;
    bool m_initialized = false;

    bool setupScenario(const MockTestScenario& scenario);
    void teardownScenario();
};

/**
 * @brief Factory for creating isolated mock test environments
 */
class MockTestEnvironmentFactory {
public:
    /**
     * @brief Create a fresh mock environment
     */
    static std::unique_ptr<MockTestRunner> createEnvironment();

    /**
     * @brief Create a mock environment with custom scenario
     */
    static std::unique_ptr<MockTestRunner> createEnvironment(const MockTestScenario& scenario);
};

} // namespace Framework
} // namespace Tests

#endif // MOCKTESTRUNNER_H
