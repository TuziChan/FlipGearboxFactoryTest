#ifndef SIMULATEDTORQUEDEVICE_H
#define SIMULATEDTORQUEDEVICE_H

#include "src/infrastructure/devices/ITorqueSensorDevice.h"
#include "SimulationContext.h"

namespace Infrastructure {
namespace Simulation {

/**
 * @brief Simulated torque sensor device (DYN200)
 * 
 * Calculates torque, speed, and power based on motor and brake state.
 */
class SimulatedTorqueDevice : public Devices::ITorqueSensorDevice {
    Q_OBJECT

public:
    explicit SimulatedTorqueDevice(SimulationContext* context, QObject* parent = nullptr);

    bool initialize() override;
    bool readTorque(double& torqueNm) override;
    bool readSpeed(double& speedRpm) override;
    bool readPower(double& powerW) override;
    bool readAll(double& torqueNm, double& speedRpm, double& powerW) override;
    QString lastError() const override;

private:
    double calculateTorque() const;
    
    SimulationContext* m_context;
};

} // namespace Simulation
} // namespace Infrastructure

#endif // SIMULATEDTORQUEDEVICE_H
