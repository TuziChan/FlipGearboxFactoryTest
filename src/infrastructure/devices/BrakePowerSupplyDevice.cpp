#include "BrakePowerSupplyDevice.h"
#include "../bus/ModbusFrame.h"
#include <QDebug>

namespace Infrastructure {
namespace Devices {

BrakePowerSupplyDevice::BrakePowerSupplyDevice(Bus::IBusController* busController, 
                                                 uint8_t slaveId,
                                                 QObject* parent)
    : IBrakePowerDevice(parent)
    , m_busController(busController)
    , m_slaveId(slaveId)
    , m_lastError()
{
}

bool BrakePowerSupplyDevice::initialize() {
    if (!m_busController || !m_busController->isOpen()) {
        m_lastError = "Bus controller is not open";
        return false;
    }

    // Disable both channels initially
    if (!setOutputEnable(1, false) || !setOutputEnable(2, false)) {
        m_lastError = QString("Failed to initialize power supply: %1").arg(m_lastError);
        return false;
    }

    QVector<uint16_t> modeValues;
    if (readInputRegisters(0x0009, 1, modeValues)) {
        if (modeValues[0] != 1) {
            qWarning() << "Brake power supply NOT in CC mode (SW5 should be ON). Current settings will be auto-cleared by device.";
        }
    }

    qDebug() << "Brake power supply initialized, slave ID:" << m_slaveId;
    return true;
}

bool BrakePowerSupplyDevice::setCurrent(int channel, double currentA) {
    if (channel < 1 || channel > 2) {
        m_lastError = "Invalid channel number (must be 1 or 2)";
        return false;
    }

    constexpr double MAX_CURRENT_A = 5.0;
    if (currentA < 0.0 || currentA > MAX_CURRENT_A) {
        m_lastError = QString("Current %1A exceeds safety limit [0, %2A]")
                          .arg(currentA, 0, 'f', 2)
                          .arg(MAX_CURRENT_A, 0, 'f', 1);
        qWarning() << m_lastError;
        return false;
    }

    uint16_t registerAddr = getSetCurrentRegister(channel);
    
    uint16_t value = static_cast<uint16_t>(currentA * 100.0);

    if (!writeRegister(registerAddr, value)) {
        m_lastError = QString("Failed to set current on channel %1: %2").arg(channel).arg(m_lastError);
        return false;
    }

    return true;
}

bool BrakePowerSupplyDevice::writeCoil(uint16_t address, bool value) {
    QByteArray request = Bus::ModbusFrame::buildWriteSingleCoil(m_slaveId, address, value);
    QByteArray response;

    if (!m_busController->sendRequest(request, response)) {
        m_lastError = m_busController->lastError();
        return false;
    }

    if (!Bus::ModbusFrame::parseWriteSingleCoilResponse(response, address, value)) {
        m_lastError = "Invalid coil write response or CRC error";
        return false;
    }

    return true;
}

bool BrakePowerSupplyDevice::setOutputEnable(int channel, bool enable) {
    if (channel < 1 || channel > 2) {
        m_lastError = "Invalid channel number (must be 1 or 2)";
        return false;
    }

    uint16_t coilAddr = getOutputEnableCoil(channel);
    if (!writeCoil(coilAddr, enable)) {
        m_lastError = QString("Failed to set output enable on channel %1: %2").arg(channel).arg(m_lastError);
        return false;
    }

    return true;
}

bool BrakePowerSupplyDevice::readCurrent(int channel, double& currentA) {
    if (channel < 1 || channel > 2) {
        m_lastError = "Invalid channel number (must be 1 or 2)";
        return false;
    }

    uint16_t registerAddr = getReadCurrentRegister(channel);
    QVector<uint16_t> values;
    if (!readInputRegisters(registerAddr, 1, values)) {
        m_lastError = QString("Failed to read current on channel %1: %2").arg(channel).arg(m_lastError);
        return false;
    }

    // Scale: ×0.01A
    currentA = values[0] * 0.01;
    return true;
}

QString BrakePowerSupplyDevice::lastError() const {
    return m_lastError;
}

bool BrakePowerSupplyDevice::readHoldingRegisters(uint16_t address, uint16_t count, QVector<uint16_t>& values) {
    QByteArray request = Bus::ModbusFrame::buildReadHoldingRegisters(m_slaveId, address, count);
    QByteArray response;
    
    if (!m_busController->sendRequest(request, response)) {
        m_lastError = m_busController->lastError();
        return false;
    }

    if (!Bus::ModbusFrame::parseReadHoldingRegistersResponse(response, count, values)) {
        m_lastError = "Invalid read response or CRC error";
        return false;
    }

    return true;
}

bool BrakePowerSupplyDevice::readInputRegisters(uint16_t address, uint16_t count, QVector<uint16_t>& values) {
    QByteArray request = Bus::ModbusFrame::buildReadInputRegisters(m_slaveId, address, count);
    QByteArray response;

    if (!m_busController->sendRequest(request, response)) {
        m_lastError = m_busController->lastError();
        return false;
    }

    if (!Bus::ModbusFrame::parseReadInputRegistersResponse(response, count, values)) {
        m_lastError = "Invalid input register response or CRC error";
        return false;
    }

    return true;
}

bool BrakePowerSupplyDevice::writeRegister(uint16_t address, uint16_t value) {
    QByteArray request = Bus::ModbusFrame::buildWriteSingleRegister(m_slaveId, address, value);
    QByteArray response;
    
    if (!m_busController->sendRequest(request, response)) {
        m_lastError = m_busController->lastError();
        return false;
    }

    if (!Bus::ModbusFrame::parseWriteSingleRegisterResponse(response, address, value)) {
        m_lastError = "Invalid write response or CRC error";
        return false;
    }

    return true;
}

uint16_t BrakePowerSupplyDevice::getSetCurrentRegister(int channel) const {
    return (channel == 1) ? REG_CH1_SET_CURRENT : REG_CH2_SET_CURRENT;
}

uint16_t BrakePowerSupplyDevice::getOutputEnableCoil(int channel) const {
    return (channel == 1) ? COIL_CH1_OUTPUT_ENABLE : COIL_CH2_OUTPUT_ENABLE;
}

uint16_t BrakePowerSupplyDevice::getReadCurrentRegister(int channel) const {
    return (channel == 1) ? REG_CH1_READ_CURRENT : REG_CH2_READ_CURRENT;
}

bool BrakePowerSupplyDevice::setVoltage(int channel, double voltageV) {
    if (channel < 1 || channel > 2) {
        m_lastError = "Invalid channel number (must be 1 or 2)";
        return false;
    }

    constexpr double MAX_VOLTAGE_V = 24.0;
    if (voltageV < 0.0 || voltageV > MAX_VOLTAGE_V) {
        m_lastError = QString("Voltage %1V exceeds safety limit [0, %2V]")
                          .arg(voltageV, 0, 'f', 2)
                          .arg(MAX_VOLTAGE_V, 0, 'f', 1);
        qWarning() << m_lastError;
        return false;
    }

    uint16_t registerAddr = getSetVoltageRegister(channel);
    uint16_t value = static_cast<uint16_t>(voltageV * 100.0);

    if (!writeRegister(registerAddr, value)) {
        m_lastError = QString("Failed to set voltage on channel %1: %2").arg(channel).arg(m_lastError);
        return false;
    }

    return true;
}

bool BrakePowerSupplyDevice::readVoltage(int channel, double& voltageV) {
    if (channel < 1 || channel > 2) {
        m_lastError = "Invalid channel number (must be 1 or 2)";
        return false;
    }

    uint16_t registerAddr = getReadVoltageRegister(channel);
    QVector<uint16_t> values;
    if (!readInputRegisters(registerAddr, 1, values)) {
        m_lastError = QString("Failed to read voltage on channel %1: %2").arg(channel).arg(m_lastError);
        return false;
    }

    voltageV = values[0] * 0.01;
    return true;
}

bool BrakePowerSupplyDevice::readPower(int channel, double& powerW) {
    if (channel < 1 || channel > 2) {
        m_lastError = "Invalid channel number (must be 1 or 2)";
        return false;
    }

    uint16_t registerAddr = getReadPowerRegister(channel);
    QVector<uint16_t> values;
    if (!readInputRegisters(registerAddr, 1, values)) {
        m_lastError = QString("Failed to read power on channel %1: %2").arg(channel).arg(m_lastError);
        return false;
    }

    powerW = values[0] * 0.01;
    return true;
}

bool BrakePowerSupplyDevice::readMode(int channel, int& mode) {
    if (channel < 1 || channel > 2) {
        m_lastError = "Invalid channel number (must be 1 or 2)";
        return false;
    }

    QVector<uint16_t> values;
    if (!readInputRegisters(REG_MODE, 1, values)) {
        m_lastError = QString("Failed to read mode on channel %1: %2").arg(channel).arg(m_lastError);
        return false;
    }

    mode = static_cast<int>(values[0]);
    return true;
}

uint16_t BrakePowerSupplyDevice::getSetVoltageRegister(int channel) const {
    return (channel == 1) ? REG_CH1_SET_VOLTAGE : REG_CH2_SET_VOLTAGE;
}

uint16_t BrakePowerSupplyDevice::getReadVoltageRegister(int channel) const {
    return (channel == 1) ? REG_CH1_READ_VOLTAGE : REG_CH2_READ_VOLTAGE;
}

uint16_t BrakePowerSupplyDevice::getReadPowerRegister(int channel) const {
    return (channel == 1) ? REG_CH1_READ_POWER : REG_CH2_READ_POWER;
}

} // namespace Devices
} // namespace Infrastructure
