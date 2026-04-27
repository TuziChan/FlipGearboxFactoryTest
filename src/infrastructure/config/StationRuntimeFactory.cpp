#include "StationRuntimeFactory.h"
#include "../bus/ModbusRtuBusController.h"
#include "../devices/AqmdMotorDriveDevice.h"
#include "../devices/Dyn200TorqueSensorDevice.h"
#include "../devices/SingleTurnEncoderDevice.h"
#include "../devices/BrakePowerSupplyDevice.h"
#include <QDebug>

namespace Infrastructure {
namespace Config {

std::unique_ptr<StationRuntime> StationRuntimeFactory::create(const StationConfig& config) {
    auto runtime = std::make_unique<StationRuntime>();

    qDebug() << "[Startup] Creating station runtime for:" << config.stationName
             << "encoderPort=" << config.encoderConfig.portName
             << "encoderBaud=" << config.encoderConfig.baudRate
             << "encoderSlaveId=" << config.encoderConfig.slaveId
             << "encoderTimeout=" << config.encoderConfig.timeout
             << "encoderParity=" << config.encoderConfig.parity
             << "encoderStopBits=" << config.encoderConfig.stopBits
             << "encoderEnabled=" << config.encoderConfig.enabled
             << "encoderResolution=" << config.encoderConfig.encoderResolution
             << "encoderCommMode=" << config.encoderConfig.communicationMode;

    runtime->m_aqmdBus = std::make_unique<Bus::ModbusRtuBusController>();
    runtime->m_aqmdBusConfig = {config.aqmdConfig.portName, config.aqmdConfig.baudRate, config.aqmdConfig.slaveId, config.aqmdConfig.timeout, config.aqmdConfig.parity, config.aqmdConfig.stopBits};

    runtime->m_dyn200Bus = std::make_unique<Bus::ModbusRtuBusController>();
    runtime->m_dyn200BusConfig = {config.dyn200Config.portName, config.dyn200Config.baudRate, config.dyn200Config.slaveId, config.dyn200Config.timeout, config.dyn200Config.parity, config.dyn200Config.stopBits};

    runtime->m_encoderBus = std::make_unique<Bus::ModbusRtuBusController>();
    runtime->m_encoderBusConfig = {config.encoderConfig.portName, config.encoderConfig.baudRate, config.encoderConfig.slaveId, config.encoderConfig.timeout, config.encoderConfig.parity, config.encoderConfig.stopBits};

    runtime->m_brakeBus = std::make_unique<Bus::ModbusRtuBusController>();
    runtime->m_brakeBusConfig = {config.brakeConfig.portName, config.brakeConfig.baudRate, config.brakeConfig.slaveId, config.brakeConfig.timeout, config.brakeConfig.parity, config.brakeConfig.stopBits};

    if (config.aqmdConfig.enabled) {
        runtime->m_motor = std::make_unique<Devices::AqmdMotorDriveDevice>(
            runtime->m_aqmdBus.get(),
            config.aqmdConfig.slaveId,
            runtime.get()
        );
    }

    if (config.dyn200Config.enabled) {
        runtime->m_torque = std::make_unique<Devices::Dyn200TorqueSensorDevice>(
            runtime->m_dyn200Bus.get(),
            config.dyn200Config.slaveId,
            config.dyn200Config.communicationMode,
            runtime.get()
        );
    }

    if (config.encoderConfig.enabled) {
        runtime->m_encoder = std::make_unique<Devices::SingleTurnEncoderDevice>(
            runtime->m_encoderBus.get(),
            config.encoderConfig.slaveId,
            config.encoderConfig.encoderResolution,
            config.encoderConfig.communicationMode,
            20,
            runtime.get()
        );
    }

    if (config.brakeConfig.enabled) {
        runtime->m_brake = std::make_unique<Devices::BrakePowerSupplyDevice>(
            runtime->m_brakeBus.get(),
            config.brakeConfig.slaveId,
            runtime.get()
        );
    }

    runtime->m_testEngine = std::make_unique<Domain::GearboxTestEngine>(runtime.get());
    runtime->m_brakeChannel = config.brakeChannel;
    runtime->m_testEngine->setBrakeChannel(config.brakeChannel);
    runtime->m_testEngine->setDevices(
        runtime->m_motor.get(),
        runtime->m_torque.get(),
        runtime->m_encoder.get(),
        runtime->m_brake.get()
    );

    runtime->m_acquisitionScheduler = std::make_unique<Acquisition::AcquisitionScheduler>(runtime.get());
    if (runtime->m_motor) {
        runtime->m_acquisitionScheduler->setMotorDevice(runtime->m_motor.get(), config.aqmdConfig.pollIntervalUs);
    }
    if (runtime->m_torque) {
        runtime->m_acquisitionScheduler->setTorqueDevice(runtime->m_torque.get(), config.dyn200Config.pollIntervalUs);
    }
    if (runtime->m_encoder) {
        runtime->m_acquisitionScheduler->setEncoderDevice(runtime->m_encoder.get(), config.encoderConfig.pollIntervalUs);
    }
    if (runtime->m_brake) {
        runtime->m_acquisitionScheduler->setBrakeDevice(runtime->m_brake.get(), config.brakeChannel, config.brakeConfig.pollIntervalUs);
    }
    runtime->m_testEngine->setAcquisitionScheduler(runtime->m_acquisitionScheduler.get());

    qDebug() << "Station runtime created successfully";
    return runtime;
}

} // namespace Config
} // namespace Infrastructure
