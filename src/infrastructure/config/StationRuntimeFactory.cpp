#include "StationRuntimeFactory.h"
#include "../bus/ModbusRtuBusController.h"
#include "../devices/AqmdMotorDriveDevice.h"
#include "../devices/Dyn200TorqueSensorDevice.h"
#include "../devices/SingleTurnEncoderDevice.h"
#include "../devices/BrakePowerSupplyDevice.h"
#include "../simulation/SimulationContext.h"
#include "../simulation/SimulatedBusController.h"
#include "../simulation/SimulatedMotorDevice.h"
#include "../simulation/SimulatedTorqueDevice.h"
#include "../simulation/SimulatedEncoderDevice.h"
#include "../simulation/SimulatedBrakeDevice.h"
#include <QDebug>

namespace Infrastructure {
namespace Config {

std::unique_ptr<StationRuntime> StationRuntimeFactory::create(const StationConfig& config, bool mockMode) {
    auto runtime = std::make_unique<StationRuntime>();

    qDebug() << "Creating station runtime for:" << config.stationName
             << (mockMode ? "(mock mode)" : "(real hardware)");

    if (mockMode) {
        // --- Mock mode: create simulated devices ---
        auto simContext = std::make_shared<Simulation::SimulationContext>();
        runtime->m_simulationContext = simContext;

        runtime->m_aqmdBus = std::make_unique<Simulation::SimulatedBusController>();
        runtime->m_dyn200Bus = std::make_unique<Simulation::SimulatedBusController>();
        runtime->m_encoderBus = std::make_unique<Simulation::SimulatedBusController>();
        runtime->m_brakeBus = std::make_unique<Simulation::SimulatedBusController>();

        // Fill bus configs with dummy values (still needed for initialize())
        runtime->m_aqmdBusConfig = {"SIM_AQMD", 9600, 1, 1000, "None", 1};
        runtime->m_dyn200BusConfig = {"SIM_DYN200", 9600, 1, 1000, "None", 1};
        runtime->m_encoderBusConfig = {"SIM_ENCODER", 9600, 1, 1000, "None", 1};
        runtime->m_brakeBusConfig = {"SIM_BRAKE", 9600, 1, 1000, "None", 1};

        if (config.aqmdConfig.enabled) {
            runtime->m_motor = std::make_unique<Simulation::SimulatedMotorDevice>(
                simContext.get(), runtime.get());
        }

        if (config.dyn200Config.enabled) {
            runtime->m_torque = std::make_unique<Simulation::SimulatedTorqueDevice>(
                simContext.get(), runtime.get());
        }

        if (config.encoderConfig.enabled) {
            runtime->m_encoder = std::make_unique<Simulation::SimulatedEncoderDevice>(
                simContext.get(), runtime.get());
        }

        if (config.brakeConfig.enabled) {
            runtime->m_brake = std::make_unique<Simulation::SimulatedBrakeDevice>(
                simContext.get(), runtime.get());
        }
    } else {
        // --- Real hardware mode ---
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
                20, // Default auto-report interval: 20ms
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
    }

    // --- Common setup (both modes) ---
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
