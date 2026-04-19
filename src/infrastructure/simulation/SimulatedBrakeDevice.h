#ifndef SIMULATEDBRAKEDEVICE_H
#define SIMULATEDBRAKEDEVICE_H

#include "src/infrastructure/devices/IBrakePowerDevice.h"
#include "SimulationContext.h"
#include <QMap>

namespace Infrastructure {
namespace Simulation {

/**
 * @brief Simulated brake power supply device
 * 
 * Records and echoes back brake current/voltage settings.
 */
class SimulatedBrakeDevice : public Devices::IBrakePowerDevice {
    Q_OBJECT

public:
    explicit SimulatedBrakeDevice(SimulationContext* context, QObject* parent = nullptr);

    bool initialize() override;
    bool setCurrent(int channel, double currentA) override;
    bool setOutputEnable(int channel, bool enable) override;
    bool readCurrent(int channel, double& currentA) override;
    bool setVoltage(int channel, double voltageV) override;
    bool readVoltage(int channel, double& voltageV) override;
    bool readPower(int channel, double& powerW) override;
    bool readMode(int channel, int& mode) override;
    bool setBrakeMode(int channel, const QString& mode) override;
    QString lastError() const override;

private:
    SimulationContext* m_context;
    QMap<int, double> m_channelCurrents;
    QMap<int, double> m_channelVoltages;
    QMap<int, bool> m_channelOutputs;
    QMap<int, int> m_channelModes; // 0=CC, 1=CV
};

} // namespace Simulation
} // namespace Infrastructure

#endif // SIMULATEDBRAKEDEVICE_H
