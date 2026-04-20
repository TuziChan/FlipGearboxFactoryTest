#include "MockModbusDevice.h"
#include <QThread>

namespace Infrastructure {
namespace Simulation {

MockModbusDevice::MockModbusDevice(uint8_t slaveId, QObject* parent)
    : QObject(parent)
    , m_slaveId(slaveId)
{
}

void MockModbusDevice::setErrorInjection(const ErrorInjectionConfig& config) {
    m_errorConfig = config;
}

void MockModbusDevice::setHoldingRegister(uint16_t address, uint16_t value) {
    m_holdingRegisters[address] = value;
}

void MockModbusDevice::setInputRegister(uint16_t address, uint16_t value) {
    m_inputRegisters[address] = value;
}

void MockModbusDevice::setCoil(uint16_t address, bool value) {
    m_coils[address] = value;
}

uint16_t MockModbusDevice::getHoldingRegister(uint16_t address) const {
    return m_holdingRegisters.value(address, 0);
}

uint16_t MockModbusDevice::getInputRegister(uint16_t address) const {
    return m_inputRegisters.value(address, 0);
}

bool MockModbusDevice::getCoil(uint16_t address) const {
    return m_coils.value(address, false);
}

QByteArray MockModbusDevice::processRequest(const QByteArray& request) {
    // Verify minimum frame size
    if (request.size() < 4) {
        return QByteArray();
    }

    // Check slave ID
    uint8_t slaveId = static_cast<uint8_t>(request[0]);
    if (slaveId != m_slaveId) {
        return QByteArray();
    }

    // Verify CRC
    if (!Bus::ModbusFrame::verifyCRC(request)) {
        return QByteArray();
    }

    // Inject timeout error
    if (m_errorConfig.enabled && shouldInjectError(m_errorConfig.timeoutRate)) {
        return QByteArray(); // No response
    }

    // Inject delay
    if (m_errorConfig.enabled && shouldInjectError(m_errorConfig.delayRate)) {
        int delay = QRandomGenerator::global()->bounded(
            m_errorConfig.minDelayMs, m_errorConfig.maxDelayMs + 1);
        QThread::msleep(delay);
    }

    // Update dynamic registers before processing
    updateDynamicRegisters();

    uint8_t functionCode = static_cast<uint8_t>(request[1]);
    QByteArray response;

    // Inject exception error
    if (m_errorConfig.enabled && shouldInjectError(m_errorConfig.exceptionRate)) {
        response = buildExceptionResponse(functionCode, 0x04); // Slave device failure
    } else {
        // Process function code
        switch (functionCode) {
        case 0x01:
            response = handleReadCoils(request);
            break;
        case 0x03:
            response = handleReadHoldingRegisters(request);
            break;
        case 0x04:
            response = handleReadInputRegisters(request);
            break;
        case 0x05:
            response = handleWriteSingleCoil(request);
            break;
        case 0x06:
            response = handleWriteSingleRegister(request);
            break;
        case 0x10:
            response = handleWriteMultipleRegisters(request);
            break;
        default:
            response = buildExceptionResponse(functionCode, 0x01); // Illegal function
            break;
        }
    }

    // Inject CRC error
    if (m_errorConfig.enabled && shouldInjectError(m_errorConfig.crcErrorRate)) {
        response = corruptCRC(response);
    }

    return response;
}

QByteArray MockModbusDevice::handleReadCoils(const QByteArray& request) {
    if (request.size() < 8) {
        return buildExceptionResponse(0x01, 0x03); // Illegal data value
    }

    uint16_t startAddress = (static_cast<uint8_t>(request[2]) << 8) | static_cast<uint8_t>(request[3]);
    uint16_t count = (static_cast<uint8_t>(request[4]) << 8) | static_cast<uint8_t>(request[5]);

    if (count == 0 || count > 2000) {
        return buildExceptionResponse(0x01, 0x03); // Illegal data value
    }

    QByteArray response;
    response.append(static_cast<char>(m_slaveId));
    response.append(static_cast<char>(0x01));

    int byteCount = (count + 7) / 8;
    response.append(static_cast<char>(byteCount));

    for (int i = 0; i < byteCount; ++i) {
        uint8_t byte = 0;
        for (int bit = 0; bit < 8 && (i * 8 + bit) < count; ++bit) {
            if (getCoil(startAddress + i * 8 + bit)) {
                byte |= (1 << bit);
            }
        }
        response.append(static_cast<char>(byte));
    }

    uint16_t crc = Bus::ModbusFrame::calculateCRC16(response);
    response.append(static_cast<char>(crc & 0xFF));
    response.append(static_cast<char>((crc >> 8) & 0xFF));

    return response;
}

QByteArray MockModbusDevice::handleReadHoldingRegisters(const QByteArray& request) {
    if (request.size() < 8) {
        return buildExceptionResponse(0x03, 0x03); // Illegal data value
    }

    uint16_t startAddress = (static_cast<uint8_t>(request[2]) << 8) | static_cast<uint8_t>(request[3]);
    uint16_t count = (static_cast<uint8_t>(request[4]) << 8) | static_cast<uint8_t>(request[5]);

    if (count == 0 || count > 125) {
        return buildExceptionResponse(0x03, 0x03); // Illegal data value
    }

    QByteArray response;
    response.append(static_cast<char>(m_slaveId));
    response.append(static_cast<char>(0x03));
    response.append(static_cast<char>(count * 2));

    for (uint16_t i = 0; i < count; ++i) {
        uint16_t value = getHoldingRegister(startAddress + i);
        response.append(static_cast<char>((value >> 8) & 0xFF));
        response.append(static_cast<char>(value & 0xFF));
    }

    uint16_t crc = Bus::ModbusFrame::calculateCRC16(response);
    response.append(static_cast<char>(crc & 0xFF));
    response.append(static_cast<char>((crc >> 8) & 0xFF));

    return response;
}

QByteArray MockModbusDevice::handleReadInputRegisters(const QByteArray& request) {
    if (request.size() < 8) {
        return buildExceptionResponse(0x04, 0x03); // Illegal data value
    }

    uint16_t startAddress = (static_cast<uint8_t>(request[2]) << 8) | static_cast<uint8_t>(request[3]);
    uint16_t count = (static_cast<uint8_t>(request[4]) << 8) | static_cast<uint8_t>(request[5]);

    if (count == 0 || count > 125) {
        return buildExceptionResponse(0x04, 0x03); // Illegal data value
    }

    QByteArray response;
    response.append(static_cast<char>(m_slaveId));
    response.append(static_cast<char>(0x04));
    response.append(static_cast<char>(count * 2));

    for (uint16_t i = 0; i < count; ++i) {
        uint16_t value = getInputRegister(startAddress + i);
        response.append(static_cast<char>((value >> 8) & 0xFF));
        response.append(static_cast<char>(value & 0xFF));
    }

    uint16_t crc = Bus::ModbusFrame::calculateCRC16(response);
    response.append(static_cast<char>(crc & 0xFF));
    response.append(static_cast<char>((crc >> 8) & 0xFF));

    return response;
}

QByteArray MockModbusDevice::handleWriteSingleCoil(const QByteArray& request) {
    if (request.size() < 8) {
        return buildExceptionResponse(0x05, 0x03); // Illegal data value
    }

    uint16_t address = (static_cast<uint8_t>(request[2]) << 8) | static_cast<uint8_t>(request[3]);
    uint16_t value = (static_cast<uint8_t>(request[4]) << 8) | static_cast<uint8_t>(request[5]);

    bool coilValue = (value == 0xFF00);
    setCoil(address, coilValue);
    onCoilWrite(address, coilValue);

    // Echo request as response
    return request;
}

QByteArray MockModbusDevice::handleWriteSingleRegister(const QByteArray& request) {
    if (request.size() < 8) {
        return buildExceptionResponse(0x06, 0x03); // Illegal data value
    }

    uint16_t address = (static_cast<uint8_t>(request[2]) << 8) | static_cast<uint8_t>(request[3]);
    uint16_t value = (static_cast<uint8_t>(request[4]) << 8) | static_cast<uint8_t>(request[5]);

    setHoldingRegister(address, value);
    onRegisterWrite(address, value);

    // Echo request as response
    return request;
}

QByteArray MockModbusDevice::handleWriteMultipleRegisters(const QByteArray& request) {
    if (request.size() < 9) {
        return buildExceptionResponse(0x10, 0x03); // Illegal data value
    }

    uint16_t startAddress = (static_cast<uint8_t>(request[2]) << 8) | static_cast<uint8_t>(request[3]);
    uint16_t count = (static_cast<uint8_t>(request[4]) << 8) | static_cast<uint8_t>(request[5]);
    uint8_t byteCount = static_cast<uint8_t>(request[6]);

    if (count == 0 || count > 123 || byteCount != count * 2 || request.size() < 9 + byteCount) {
        return buildExceptionResponse(0x10, 0x03); // Illegal data value
    }

    for (uint16_t i = 0; i < count; ++i) {
        uint16_t value = (static_cast<uint8_t>(request[7 + i * 2]) << 8) | 
                         static_cast<uint8_t>(request[8 + i * 2]);
        setHoldingRegister(startAddress + i, value);
        onRegisterWrite(startAddress + i, value);
    }

    QByteArray response;
    response.append(static_cast<char>(m_slaveId));
    response.append(static_cast<char>(0x10));
    response.append(static_cast<char>((startAddress >> 8) & 0xFF));
    response.append(static_cast<char>(startAddress & 0xFF));
    response.append(static_cast<char>((count >> 8) & 0xFF));
    response.append(static_cast<char>(count & 0xFF));

    uint16_t crc = Bus::ModbusFrame::calculateCRC16(response);
    response.append(static_cast<char>(crc & 0xFF));
    response.append(static_cast<char>((crc >> 8) & 0xFF));

    return response;
}

QByteArray MockModbusDevice::buildExceptionResponse(uint8_t functionCode, uint8_t exceptionCode) {
    QByteArray response;
    response.append(static_cast<char>(m_slaveId));
    response.append(static_cast<char>(functionCode | 0x80));
    response.append(static_cast<char>(exceptionCode));

    uint16_t crc = Bus::ModbusFrame::calculateCRC16(response);
    response.append(static_cast<char>(crc & 0xFF));
    response.append(static_cast<char>((crc >> 8) & 0xFF));

    return response;
}

bool MockModbusDevice::shouldInjectError(double rate) {
    if (rate <= 0.0) return false;
    if (rate >= 1.0) return true;
    return QRandomGenerator::global()->generateDouble() < rate;
}

QByteArray MockModbusDevice::corruptCRC(const QByteArray& frame) {
    if (frame.size() < 2) return frame;
    
    QByteArray corrupted = frame;
    corrupted[frame.size() - 1] = static_cast<char>(~static_cast<uint8_t>(frame[frame.size() - 1]));
    return corrupted;
}

void MockModbusDevice::onRegisterWrite(uint16_t address, uint16_t value) {
    // Default: no action
    Q_UNUSED(address);
    Q_UNUSED(value);
}

void MockModbusDevice::onCoilWrite(uint16_t address, bool value) {
    // Default: no action
    Q_UNUSED(address);
    Q_UNUSED(value);
}

void MockModbusDevice::updateDynamicRegisters() {
    // Default: no action
}

} // namespace Simulation
} // namespace Infrastructure
