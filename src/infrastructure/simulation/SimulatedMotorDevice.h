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

    // Magnet detection configuration
    static constexpr double MAGNET_POSITIONS[3] = {3.0, 49.0, 113.5}; // Degrees (absolute)
    static constexpr double DETECTION_WINDOW = 2.0; // ±2 degrees
    bool m_magnetLastState[3] = {false, false, false};
    int m_magnetPassCounts[3] = {0, 0, 0};

    // Helper method for angle-based magnet detection
    bool isAngleInWindow(double angle, double targetAngle, double window) const;
    bool isMagnetCrossed(double fromAngle, double toAngle, double magnetPos) const;
    double normalizeAngle(double angle) const;
};

} // namespace Simulation
} // namespace Infrastructure

#endif // SIMULATEDMOTORDEVICE_H
