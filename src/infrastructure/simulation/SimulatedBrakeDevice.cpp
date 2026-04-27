#include "SimulatedBrakeDevice.h"

namespace Infrastructure {
namespace Simulation {

SimulatedBrakeDevice::SimulatedBrakeDevice(SimulationContext* context, QObject* parent)
    : IBrakePowerDevice(parent)
    , m_context(context)
{
    // Initialize default values for channels
    m_channelCurrents[1] = 0.0;
    m_channelCurrents[2] = 0.0;
    m_channelVoltages[1] = 0.0;
    m_channelVoltages[2] = 0.0;
    m_channelOutputs[1] = false;
    m_channelOutputs[2] = false;
    m_channelModes[1] = 0; // CC mode
    m_channelModes[2] = 0; // CC mode
}

bool SimulatedBrakeDevice::initialize() {
    return true;
}

bool SimulatedBrakeDevice::setCurrent(int channel, double currentA) {
    if (!m_context) return false;

    // Safety limit matching real device (BrakePowerSupplyDevice: MAX_CURRENT_A = 5.0)
    constexpr double MAX_CURRENT_A = 5.0;
    if (currentA < 0.0 || currentA > MAX_CURRENT_A) {
        m_lastError = QString("Current %1A exceeds safety limit [0, %2A]")
                          .arg(currentA, 0, 'f', 2)
                          .arg(MAX_CURRENT_A, 0, 'f', 1);
        return false;
    }

    m_channelCurrents[channel] = currentA;

    // Update context with channel 1 current (primary brake channel)
    if (channel == 1) {
        m_context->setBrakeCurrent(currentA);
    }

    return true;
}

bool SimulatedBrakeDevice::setOutputEnable(int channel, bool enable) {
    if (!m_context) return false;

    m_channelOutputs[channel] = enable;

    // Update context with channel 1 output state
    if (channel == 1) {
        m_context->setBrakeOutputEnabled(enable);
    }

    return true;
}

bool SimulatedBrakeDevice::readCurrent(int channel, double& currentA) {
    if (!m_context) return false;

    m_context->incrementTickCount();

    currentA = m_channelCurrents.value(channel, 0.0);
    return true;
}

bool SimulatedBrakeDevice::setVoltage(int channel, double voltageV) {
    if (!m_context) return false;

    // Safety limit matching real device (BrakePowerSupplyDevice: MAX_VOLTAGE_V = 24.0)
    constexpr double MAX_VOLTAGE_V = 24.0;
    if (voltageV < 0.0 || voltageV > MAX_VOLTAGE_V) {
        m_lastError = QString("Voltage %1V exceeds safety limit [0, %2V]")
                          .arg(voltageV, 0, 'f', 2)
                          .arg(MAX_VOLTAGE_V, 0, 'f', 1);
        return false;
    }

    m_channelVoltages[channel] = voltageV;

    // Update context with channel 1 voltage
    if (channel == 1) {
        m_context->setBrakeVoltage(voltageV);
    }

    return true;
}

bool SimulatedBrakeDevice::readVoltage(int channel, double& voltageV) {
    if (!m_context) return false;

    m_context->incrementTickCount();

    voltageV = m_channelVoltages.value(channel, 0.0);
    return true;
}

bool SimulatedBrakeDevice::readPower(int channel, double& powerW) {
    if (!m_context) return false;

    m_context->incrementTickCount();

    // Power = Current * Voltage
    double current = m_channelCurrents.value(channel, 0.0);
    double voltage = m_channelVoltages.value(channel, 0.0);
    powerW = current * voltage;

    return true;
}

bool SimulatedBrakeDevice::readMode(int channel, int& mode) {
    if (!m_context) return false;

    m_context->incrementTickCount();

    mode = m_channelModes.value(channel, 0);
    return true;
}

bool SimulatedBrakeDevice::setBrakeMode(int channel, const QString& mode) {
    if (!m_context) return false;

    // Set mode: "CC" = 0, "CV" = 1
    if (mode == "CV") {
        m_channelModes[channel] = 1;
    } else {
        m_channelModes[channel] = 0;
    }

    return true;
}

QString SimulatedBrakeDevice::lastError() const {
    return m_lastError;
}

} // namespace Simulation
} // namespace Infrastructure
