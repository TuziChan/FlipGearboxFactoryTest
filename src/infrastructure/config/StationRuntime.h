#ifndef STATIONRUNTIME_H
#define STATIONRUNTIME_H

#include "../bus/IBusController.h"
#include "../devices/IMotorDriveDevice.h"
#include "../devices/ITorqueSensorDevice.h"
#include "../devices/IEncoderDevice.h"
#include "../devices/IBrakePowerDevice.h"
#include "../simulation/SimulationContext.h"
#include "../acquisition/AcquisitionScheduler.h"
#include "../../domain/GearboxTestEngine.h"
#include <QObject>
#include <memory>

namespace Infrastructure {
namespace Config {

/**
 * @brief Runtime assembly of all devices and test engine
 */
class StationRuntime : public QObject {
    Q_OBJECT

public:
    explicit StationRuntime(QObject* parent = nullptr);
    ~StationRuntime() override;

    Devices::IMotorDriveDevice* motor() const { return m_motor.get(); }
    Devices::ITorqueSensorDevice* torque() const { return m_torque.get(); }
    Devices::IEncoderDevice* encoder() const { return m_encoder.get(); }
    Devices::IBrakePowerDevice* brake() const { return m_brake.get(); }
    Bus::IBusController* aqmdBus() const { return m_aqmdBus.get(); }
    Bus::IBusController* dyn200Bus() const { return m_dyn200Bus.get(); }
    Bus::IBusController* encoderBus() const { return m_encoderBus.get(); }
    Bus::IBusController* brakeBus() const { return m_brakeBus.get(); }
    Domain::GearboxTestEngine* testEngine() const { return m_testEngine.get(); }
    Acquisition::AcquisitionScheduler* acquisitionScheduler() const { return m_acquisitionScheduler.get(); }
    int brakeChannel() const { return m_brakeChannel; }
    Q_INVOKABLE bool isInitialized() const { return m_initialized; }

    Q_INVOKABLE bool initialize();
    void shutdown();

    QString lastError() const { return m_lastError; }

private:
    friend class StationRuntimeFactory;

    std::unique_ptr<Bus::IBusController> m_aqmdBus;
    std::unique_ptr<Bus::IBusController> m_dyn200Bus;
    std::unique_ptr<Bus::IBusController> m_encoderBus;
    std::unique_ptr<Bus::IBusController> m_brakeBus;

    struct BusConfig {
        QString portName;
        int baudRate;
        int slaveId;
        int timeoutMs;
        QString parity;
        int stopBits;
    };
    BusConfig m_aqmdBusConfig;
    BusConfig m_dyn200BusConfig;
    BusConfig m_encoderBusConfig;
    BusConfig m_brakeBusConfig;

    std::unique_ptr<Devices::IMotorDriveDevice> m_motor;
    std::unique_ptr<Devices::ITorqueSensorDevice> m_torque;
    std::unique_ptr<Devices::IEncoderDevice> m_encoder;
    std::unique_ptr<Devices::IBrakePowerDevice> m_brake;
    std::unique_ptr<Domain::GearboxTestEngine> m_testEngine;
    std::shared_ptr<Simulation::SimulationContext> m_simulationContext;
    std::unique_ptr<Acquisition::AcquisitionScheduler> m_acquisitionScheduler;

    QString m_lastError;
    int m_brakeChannel = 1;
    bool m_initialized = false;
    bool initializeBus(const QString& displayName,
                       const BusConfig& config,
                       const std::unique_ptr<Bus::IBusController>& bus,
                       bool enabled);
};

} // namespace Config
} // namespace Infrastructure

#endif // STATIONRUNTIME_H
