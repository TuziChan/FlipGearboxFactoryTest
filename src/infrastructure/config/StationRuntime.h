#ifndef STATIONRUNTIME_H
#define STATIONRUNTIME_H

#include "../bus/ModbusRtuBusController.h"
#include "../devices/AqmdMotorDriveDevice.h"
#include "../devices/Dyn200TorqueSensorDevice.h"
#include "../devices/SingleTurnEncoderDevice.h"
#include "../devices/BrakePowerSupplyDevice.h"
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

    Devices::AqmdMotorDriveDevice* motor() const { return m_motor.get(); }
    Devices::Dyn200TorqueSensorDevice* torque() const { return m_torque.get(); }
    Devices::SingleTurnEncoderDevice* encoder() const { return m_encoder.get(); }
    Devices::BrakePowerSupplyDevice* brake() const { return m_brake.get(); }
    Domain::GearboxTestEngine* testEngine() const { return m_testEngine.get(); }
    int brakeChannel() const { return m_brakeChannel; }
    bool isInitialized() const { return m_initialized; }

    bool initialize();
    void shutdown();

    QString lastError() const { return m_lastError; }

private:
    friend class StationRuntimeFactory;

    std::unique_ptr<Bus::ModbusRtuBusController> m_aqmdBus;
    std::unique_ptr<Bus::ModbusRtuBusController> m_dyn200Bus;
    std::unique_ptr<Bus::ModbusRtuBusController> m_encoderBus;
    std::unique_ptr<Bus::ModbusRtuBusController> m_brakeBus;

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

    std::unique_ptr<Devices::AqmdMotorDriveDevice> m_motor;
    std::unique_ptr<Devices::Dyn200TorqueSensorDevice> m_torque;
    std::unique_ptr<Devices::SingleTurnEncoderDevice> m_encoder;
    std::unique_ptr<Devices::BrakePowerSupplyDevice> m_brake;
    std::unique_ptr<Domain::GearboxTestEngine> m_testEngine;

    QString m_lastError;
    int m_brakeChannel = 1;
    bool m_initialized = false;
    bool initializeBus(const QString& displayName,
                       const BusConfig& config,
                       const std::unique_ptr<Bus::ModbusRtuBusController>& bus,
                       bool enabled);
};

} // namespace Config
} // namespace Infrastructure

#endif // STATIONRUNTIME_H
