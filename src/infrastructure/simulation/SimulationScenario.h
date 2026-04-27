#ifndef SIMULATIONSCENARIO_H
#define SIMULATIONSCENARIO_H

#include "SimulationContext.h"
#include <QObject>
#include <QElapsedTimer>
#include <QString>
#include <QVariantMap>
#include <QJsonObject>
#include <QVector>
#include <functional>
#include <memory>

namespace Infrastructure {
namespace Simulation {

class HardwareSimulationHarness;
class EnhancedSimulationContext;

/**
 * @brief Defines a simulation scenario for automated testing
 *
 * A scenario describes:
 * - Initial physical state (motor speed, brake load, encoder position)
 * - Fault injection points (bus errors, sensor faults, timeouts)
 * - Expected behavior / pass criteria
 * - Step-by-step timeline of actions
 */
struct SimulationScenario {
    QString name;
    QString description;
    QString category; // e.g. "normal", "fault_injection", "boundary"

    // Initial state
    struct InitialState {
        double motorDutyCycle = 0.0;
        SimulationContext::MotorDirection motorDirection = SimulationContext::MotorDirection::Stopped;
        double brakeCurrentA = 0.0;
        bool brakeEnabled = false;
        double encoderAngleDeg = 0.0;
        double encoderZeroOffset = 0.0;
    };
    InitialState initialState;

    // Fault injection schedule: tick -> fault config
    struct FaultEvent {
        uint64_t tick;
        QString target; // "bus", "encoder", "torque", "speed"
        int faultCode;  // Maps to FaultMode or FaultType
        QVariantMap params;
    };
    QVector<FaultEvent> faultSchedule;

    // Action schedule: tick -> action
    struct ActionEvent {
        uint64_t tick;
        QString action; // "setMotor", "setBrake", "setEncoderZero", "advanceTicks"
        QVariantMap params;
    };
    QVector<ActionEvent> actionSchedule;

    // Pass criteria
    struct PassCriteria {
        double maxEncoderAngleError = 1.0;    // degrees
        double maxTorqueError = 0.5;          // Nm
        double maxSpeedError = 10.0;          // RPM
        uint64_t maxTicks = 1000;
        bool expectBusFailure = false;
        bool expectSensorFault = false;
    };
    PassCriteria passCriteria;

    // Utility
    QJsonObject toJson() const;
    static SimulationScenario fromJson(const QJsonObject& obj);
    static SimulationScenario fromJsonFile(const QString& filePath);
};

/**
 * @brief Scenario executor that runs a SimulationScenario against a harness
 */
class ScenarioExecutor : public QObject {
    Q_OBJECT

public:
    explicit ScenarioExecutor(HardwareSimulationHarness* harness, QObject* parent = nullptr);

    struct ExecutionResult {
        bool passed = false;
        QString failureReason;
        qint64 durationMs = 0;
        uint64_t ticksExecuted = 0;
        QVector<QString> eventsLog;
        QVariantMap metrics;
    };

    ExecutionResult execute(const SimulationScenario& scenario);

    // Step-by-step execution for debugging
    void prepare(const SimulationScenario& scenario);
    bool step(); // Execute next tick/action, returns false when done
    bool isFinished() const;
    ExecutionResult finalize();

signals:
    void tickAdvanced(uint64_t tick);
    void faultInjected(const QString& target, int code);
    void actionExecuted(const QString& action, const QVariantMap& params);
    void criteriaChecked(bool passed, const QString& detail);

private:
    HardwareSimulationHarness* m_harness = nullptr;
    SimulationScenario m_currentScenario;
    uint64_t m_currentTick = 0;
    bool m_prepared = false;
    bool m_finished = false;
    QElapsedTimer m_timer;
    QVector<QString> m_eventsLog;

    void applyInitialState(const SimulationScenario::InitialState& state);
    void processFaultEvents(uint64_t tick);
    void processActionEvents(uint64_t tick);
    bool checkPassCriteria(const SimulationScenario::PassCriteria& criteria);
};

} // namespace Simulation
} // namespace Infrastructure

#endif // SIMULATIONSCENARIO_H
