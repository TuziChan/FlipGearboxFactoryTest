#ifndef SIMULATEDMOTORDEVICE_H
#define SIMULATEDMOTORDEVICE_H

#include "src/infrastructure/devices/IMotorDriveDevice.h"
#include "SimulationContext.h"

namespace Infrastructure {
namespace Simulation {

/**
 * @brief Simulated motor drive device (AQMD)
 * 
 * Simulates motor control and magnet detection for homing operations.
 */
class SimulatedMotorDevice : public Devices::IMotorDriveDevice {
    Q_OBJECT

public:
    explicit SimulatedMotorDevice(SimulationContext* context, QObject* parent = nullptr);

    bool initialize() override;
    bool setMotor(Direction direction, double dutyCyclePercent) override;
    bool brake() override;
    bool coast() override;
    bool readCurrent(double& currentA) override;
    bool readAI1Level(bool& level) override;
    QString lastError() const override;

private:
    SimulationContext* m_context;
    uint64_t m_ai1TransitionTick;
    static constexpr uint64_t AI1_TRANSITION_DELAY = 75; // Ticks before magnet detected
};

} // namespace Simulation
} // namespace Infrastructure

#endif // SIMULATEDMOTORDEVICE_H
