#ifndef HARDWARESIMULATIONHARNESS_H
#define HARDWARESIMULATIONHARNESS_H

#include "SimulationContext.h"
#include "EnhancedSimulationContext.h"
#include "SimulatedBusController.h"
#include "SimulatedBusControllerWithFaults.h"
#include "SimulatedMotorDevice.h"
#include "SimulatedTorqueDevice.h"
#include "SimulatedEncoderDevice.h"
#include "SimulatedBrakeDevice.h"
#include "../devices/IMotorDriveDevice.h"
#include "../devices/ITorqueSensorDevice.h"
#include "../devices/IEncoderDevice.h"
#include "../devices/IBrakePowerDevice.h"
#include <QObject>
#include <memory>

namespace Infrastructure {
namespace Simulation {

/**
 * @brief Unified hardware simulation harness
 *
 * Wraps all simulated devices into a single manageable unit.
 * Provides scenario-based control, fault injection, and lifecycle management.
 * This is the primary entry point for mock testing and hardware simulation.
 */
class HardwareSimulationHarness : public QObject {
    Q_OBJECT

public:
    explicit HardwareSimulationHarness(QObject* parent = nullptr);
    ~HardwareSimulationHarness() override;

    // Device accessors (return interface pointers for polymorphic use)
    Devices::IMotorDriveDevice* motorDevice() const;
    Devices::ITorqueSensorDevice* torqueDevice() const;
    Devices::IEncoderDevice* encoderDevice() const;
    Devices::IBrakePowerDevice* brakeDevice() const;
    Bus::IBusController* busController(int index) const;

    // Context accessors
    SimulationContext* simulationContext() const;
    EnhancedSimulationContext* enhancedContext() const;

    // Lifecycle
    bool initialize();
    void shutdown();
    bool isRunning() const;

    // Scenario control
    void resetToDefaults();
    void setMotorState(SimulationContext::MotorDirection direction, double dutyCycle);
    void setBrakeOutput(bool enabled, double currentA);
    void setEncoderZeroOffset(double offsetDeg);

    // Fault injection (delegates to EnhancedSimulationContext)
    void injectBusFault(int busIndex, SimulatedBusControllerWithFaults::FaultMode mode);
    void clearBusFaults();

    void injectSensorFault(EnhancedSimulationContext::FaultType encoderFault,
                           EnhancedSimulationContext::FaultType torqueFault,
                           EnhancedSimulationContext::FaultType speedFault);
    void clearSensorFaults();

    // Magnet position configuration for angle tests
    void setMagnetPositions(const QVector<double>& positions);
    QVector<double> magnetPositions() const;

    // Statistics
    struct HarnessStats {
        int busFailureCount = 0;
        int busSuccessCount = 0;
        int busTimeoutCount = 0;
        int busCrcErrorCount = 0;
        uint64_t totalTicks = 0;
        bool motorInitialized = false;
        bool torqueInitialized = false;
        bool encoderInitialized = false;
        bool brakeInitialized = false;
    };
    HarnessStats stats() const;

    // QML-friendly properties
    Q_INVOKABLE bool isInitialized() const { return m_initialized; }
    Q_INVOKABLE double currentEncoderAngle() const;
    Q_INVOKABLE double currentMotorSpeedRpm() const;
    Q_INVOKABLE double currentTorqueNm() const;

signals:
    void initialized();
    void shutdownComplete();
    void faultInjected(const QString& faultType, const QString& target);
    void stateChanged();

private:
    std::shared_ptr<EnhancedSimulationContext> m_context;

    std::unique_ptr<SimulatedBusController> m_busNormal;
    std::unique_ptr<SimulatedBusControllerWithFaults> m_busFaulty;

    std::unique_ptr<SimulatedMotorDevice> m_motor;
    std::unique_ptr<SimulatedTorqueDevice> m_torque;
    std::unique_ptr<SimulatedEncoderDevice> m_encoder;
    std::unique_ptr<SimulatedBrakeDevice> m_brake;

    bool m_initialized = false;
    bool m_useFaultyBus = false;
    int m_activeBusCount = 4;

    void createDevices();
};

} // namespace Simulation
} // namespace Infrastructure

#endif // HARDWARESIMULATIONHARNESS_H
