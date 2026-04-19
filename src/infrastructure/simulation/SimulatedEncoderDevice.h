#ifndef SIMULATEDENCODERDEVICE_H
#define SIMULATEDENCODERDEVICE_H

#include "src/infrastructure/devices/IEncoderDevice.h"
#include "SimulationContext.h"

namespace Infrastructure {
namespace Simulation {

/**
 * @brief Simulated encoder device
 * 
 * Provides angle readings based on motor movement simulation.
 */
class SimulatedEncoderDevice : public Devices::IEncoderDevice {
    Q_OBJECT

public:
    explicit SimulatedEncoderDevice(SimulationContext* context, QObject* parent = nullptr);

    bool initialize() override;
    bool readAngle(double& angleDeg) override;
    bool readVirtualMultiTurn(double& totalAngleDeg) override;
    bool readAngularVelocity(double& velocityRpm) override;
    bool setZeroPoint() override;
    QString lastError() const override;

private:
    SimulationContext* m_context;
};

} // namespace Simulation
} // namespace Infrastructure

#endif // SIMULATEDENCODERDEVICE_H
