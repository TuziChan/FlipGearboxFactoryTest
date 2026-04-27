#include "MockBrakeDevice.h"

namespace Infrastructure {
namespace Simulation {

MockBrakeDevice::MockBrakeDevice(uint8_t slaveId, QObject* parent)
    : MockModbusDevice(slaveId, parent)
{
    // Initialize holding registers (setpoints)
    setHoldingRegister(REG_CH1_SET_VOLTAGE, 0);
    setHoldingRegister(REG_CH1_SET_CURRENT, 0);
    setHoldingRegister(REG_CH2_SET_VOLTAGE, 0);
    setHoldingRegister(REG_CH2_SET_CURRENT, 0);

    // Initialize input registers (readback)
    setInputRegister(REG_CH1_READ_VOLTAGE, 0);
    setInputRegister(REG_CH1_READ_CURRENT, 0);
    setInputRegister(REG_CH1_READ_POWER, 0);
    setInputRegister(REG_CH2_READ_VOLTAGE, 0);
    setInputRegister(REG_CH2_READ_CURRENT, 0);
    setInputRegister(REG_CH2_READ_POWER, 0);
    setInputRegister(REG_MODE, 0); // Both channels in CC mode

    // Initialize coils (output enable)
    setCoil(COIL_CH1_OUTPUT_ENABLE, false);
    setCoil(COIL_CH2_OUTPUT_ENABLE, false);
}

void MockBrakeDevice::setChannelMode(int channel, int mode) {
    if (channel == 1) {
        m_channel1.mode = mode;
    } else if (channel == 2) {
        m_channel2.mode = mode;
    }

    // Update mode register
    uint16_t modeReg = (m_channel2.mode << 1) | m_channel1.mode;
    setInputRegister(REG_MODE, modeReg);
}

void MockBrakeDevice::onRegisterWrite(uint16_t address, uint16_t value) {
    switch (address) {
    case REG_CH1_SET_VOLTAGE:
        m_channel1.setVoltageV = value * 0.01;
        updateChannelOutput(1);
        break;

    case REG_CH1_SET_CURRENT:
        m_channel1.setCurrentA = value * 0.01;
        updateChannelOutput(1);
        break;

    case REG_CH2_SET_VOLTAGE:
        m_channel2.setVoltageV = value * 0.01;
        updateChannelOutput(2);
        break;

    case REG_CH2_SET_CURRENT:
        m_channel2.setCurrentA = value * 0.01;
        updateChannelOutput(2);
        break;

    default:
        break;
    }
}

void MockBrakeDevice::onCoilWrite(uint16_t address, bool value) {
    switch (address) {
    case COIL_CH1_OUTPUT_ENABLE:
        m_channel1.outputEnabled = value;
        updateChannelOutput(1);
        break;

    case COIL_CH2_OUTPUT_ENABLE:
        m_channel2.outputEnabled = value;
        updateChannelOutput(2);
        break;

    default:
        break;
    }
}

void MockBrakeDevice::updateDynamicRegisters() {
    // Update channel 1 readback
    updateChannelOutput(1);
    
    // Update channel 2 readback
    updateChannelOutput(2);
}

void MockBrakeDevice::updateChannelOutput(int channel) {
    ChannelState* state = (channel == 1) ? &m_channel1 : &m_channel2;
    
    double actualVoltageV = 0.0;
    double actualCurrentA = 0.0;
    double actualPowerW = 0.0;

    if (state->outputEnabled) {
        if (state->mode == 0) {
            // CC mode: current follows setpoint, voltage varies
            actualCurrentA = state->setCurrentA;
            actualVoltageV = state->setVoltageV; // Simplified: assume voltage reaches setpoint
            actualPowerW = actualVoltageV * actualCurrentA;
        } else {
            // CV mode: voltage follows setpoint, current varies
            actualVoltageV = state->setVoltageV;
            actualCurrentA = state->setCurrentA; // Simplified: assume current reaches setpoint
            actualPowerW = actualVoltageV * actualCurrentA;
        }
    }

    // Update input registers (×100 for voltage/current/power)
    if (channel == 1) {
        setInputRegister(REG_CH1_READ_VOLTAGE, static_cast<uint16_t>(actualVoltageV * 100.0));
        setInputRegister(REG_CH1_READ_CURRENT, static_cast<uint16_t>(actualCurrentA * 100.0));
        setInputRegister(REG_CH1_READ_POWER, static_cast<uint16_t>(actualPowerW * 100.0));
    } else {
        setInputRegister(REG_CH2_READ_VOLTAGE, static_cast<uint16_t>(actualVoltageV * 100.0));
        setInputRegister(REG_CH2_READ_CURRENT, static_cast<uint16_t>(actualCurrentA * 100.0));
        setInputRegister(REG_CH2_READ_POWER, static_cast<uint16_t>(actualPowerW * 100.0));
    }
}

} // namespace Simulation
} // namespace Infrastructure
