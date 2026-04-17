#include "ModbusFrame.h"

namespace Infrastructure {
namespace Bus {

QByteArray ModbusFrame::buildReadCoils(uint8_t slaveId, uint16_t startAddress, uint16_t count) {
    QByteArray frame;
    frame.append(static_cast<char>(slaveId));
    frame.append(static_cast<char>(FunctionCode::ReadCoils));
    frame.append(static_cast<char>((startAddress >> 8) & 0xFF));
    frame.append(static_cast<char>(startAddress & 0xFF));
    frame.append(static_cast<char>((count >> 8) & 0xFF));
    frame.append(static_cast<char>(count & 0xFF));

    const uint16_t crc = calculateCRC16(frame);
    frame.append(static_cast<char>(crc & 0xFF));
    frame.append(static_cast<char>((crc >> 8) & 0xFF));

    return frame;
}

QByteArray ModbusFrame::buildReadHoldingRegisters(uint8_t slaveId, uint16_t startAddress, uint16_t count) {
    QByteArray frame;
    frame.append(static_cast<char>(slaveId));
    frame.append(static_cast<char>(FunctionCode::ReadHoldingRegisters));
    frame.append(static_cast<char>((startAddress >> 8) & 0xFF));
    frame.append(static_cast<char>(startAddress & 0xFF));
    frame.append(static_cast<char>((count >> 8) & 0xFF));
    frame.append(static_cast<char>(count & 0xFF));
    
    uint16_t crc = calculateCRC16(frame);
    frame.append(static_cast<char>(crc & 0xFF));        // CRC low byte first
    frame.append(static_cast<char>((crc >> 8) & 0xFF)); // CRC high byte
    
    return frame;
}

QByteArray ModbusFrame::buildReadInputRegisters(uint8_t slaveId, uint16_t startAddress, uint16_t count) {
    QByteArray frame;
    frame.append(static_cast<char>(slaveId));
    frame.append(static_cast<char>(FunctionCode::ReadInputRegisters));
    frame.append(static_cast<char>((startAddress >> 8) & 0xFF));
    frame.append(static_cast<char>(startAddress & 0xFF));
    frame.append(static_cast<char>((count >> 8) & 0xFF));
    frame.append(static_cast<char>(count & 0xFF));

    const uint16_t crc = calculateCRC16(frame);
    frame.append(static_cast<char>(crc & 0xFF));
    frame.append(static_cast<char>((crc >> 8) & 0xFF));

    return frame;
}

QByteArray ModbusFrame::buildWriteSingleCoil(uint8_t slaveId, uint16_t coilAddress, bool value) {
    QByteArray frame;
    frame.append(static_cast<char>(slaveId));
    frame.append(static_cast<char>(FunctionCode::WriteSingleCoil));
    frame.append(static_cast<char>((coilAddress >> 8) & 0xFF));
    frame.append(static_cast<char>(coilAddress & 0xFF));
    frame.append(static_cast<char>(value ? 0xFF : 0x00));
    frame.append(static_cast<char>(0x00));

    const uint16_t crc = calculateCRC16(frame);
    frame.append(static_cast<char>(crc & 0xFF));
    frame.append(static_cast<char>((crc >> 8) & 0xFF));

    return frame;
}

QByteArray ModbusFrame::buildWriteSingleRegister(uint8_t slaveId, uint16_t registerAddress, uint16_t value) {
    QByteArray frame;
    frame.append(static_cast<char>(slaveId));
    frame.append(static_cast<char>(FunctionCode::WriteSingleRegister));
    frame.append(static_cast<char>((registerAddress >> 8) & 0xFF));
    frame.append(static_cast<char>(registerAddress & 0xFF));
    frame.append(static_cast<char>((value >> 8) & 0xFF));
    frame.append(static_cast<char>(value & 0xFF));
    
    uint16_t crc = calculateCRC16(frame);
    frame.append(static_cast<char>(crc & 0xFF));
    frame.append(static_cast<char>((crc >> 8) & 0xFF));
    
    return frame;
}

QByteArray ModbusFrame::buildWriteSingleRegisterSigned(uint8_t slaveId, uint16_t registerAddress, int16_t value) {
    // Reinterpret signed as unsigned for transmission
    uint16_t unsignedValue = static_cast<uint16_t>(value);
    return buildWriteSingleRegister(slaveId, registerAddress, unsignedValue);
}

bool ModbusFrame::parseReadHoldingRegistersResponse(const QByteArray& response, 
                                                     uint16_t expectedCount,
                                                     QVector<uint16_t>& outValues) {
    const int expectedLength = 3 + (expectedCount * 2) + 2;
    if (response.size() < expectedLength) {
        return false;
    }
    if (!verifyCRC(response)) {
        return false;
    }

    const uint8_t functionCode = static_cast<uint8_t>(response[1]);
    if (functionCode != static_cast<uint8_t>(FunctionCode::ReadHoldingRegisters)) {
        return false;
    }

    const uint8_t byteCount = static_cast<uint8_t>(response[2]);
    if (byteCount != expectedCount * 2) {
        return false;
    }
    outValues.clear();
    for (int i = 0; i < expectedCount; ++i) {
        const int offset = 3 + (i * 2);
        const uint16_t value = (static_cast<uint8_t>(response[offset]) << 8)
                             | static_cast<uint8_t>(response[offset + 1]);
        outValues.append(value);
    }

    return true;
}

bool ModbusFrame::parseReadInputRegistersResponse(const QByteArray& response,
                                                  uint16_t expectedCount,
                                                  QVector<uint16_t>& outValues) {
    const int expectedLength = 3 + (expectedCount * 2) + 2;
    if (response.size() < expectedLength) {
        return false;
    }

    if (!verifyCRC(response)) {
        return false;
    }

    if (static_cast<uint8_t>(response[1]) != static_cast<uint8_t>(FunctionCode::ReadInputRegisters)) {
        return false;
    }

    const uint8_t byteCount = static_cast<uint8_t>(response[2]);
    if (byteCount != expectedCount * 2) {
        return false;
    }

    outValues.clear();
    for (int i = 0; i < expectedCount; ++i) {
        const int offset = 3 + (i * 2);
        const uint16_t value = (static_cast<uint8_t>(response[offset]) << 8)
                             | static_cast<uint8_t>(response[offset + 1]);
        outValues.append(value);
    }

    return true;
}

bool ModbusFrame::parseReadCoilsResponse(const QByteArray& response,
                                         uint16_t expectedCount,
                                         QVector<bool>& outValues) {
    const int expectedByteCount = (expectedCount + 7) / 8;
    const int expectedLength = 3 + expectedByteCount + 2;
    if (response.size() < expectedLength) {
        return false;
    }

    if (!verifyCRC(response)) {
        return false;
    }

    if (static_cast<uint8_t>(response[1]) != static_cast<uint8_t>(FunctionCode::ReadCoils)) {
        return false;
    }

    if (static_cast<uint8_t>(response[2]) != expectedByteCount) {
        return false;
    }

    outValues.clear();
    for (int i = 0; i < expectedCount; ++i) {
        const int byteOffset = 3 + (i / 8);
        const int bitOffset = i % 8;
        outValues.append(((static_cast<uint8_t>(response[byteOffset]) >> bitOffset) & 0x01) != 0);
    }
    return true;
}

bool ModbusFrame::parseWriteSingleRegisterResponse(const QByteArray& response,
                                                    uint16_t expectedAddress,
                                                    uint16_t expectedValue) {
    // Response format: SlaveId(1) + FunctionCode(1) + Address(2) + Value(2) + CRC(2) = 8 bytes
    if (response.size() != 8) {
        return false;
    }
    
    if (!verifyCRC(response)) {
        return false;
    }
    
    uint8_t functionCode = static_cast<uint8_t>(response[1]);
    if (functionCode != static_cast<uint8_t>(FunctionCode::WriteSingleRegister)) {
        return false;
    }
    
    uint16_t address = (static_cast<uint8_t>(response[2]) << 8) | static_cast<uint8_t>(response[3]);
    uint16_t value = (static_cast<uint8_t>(response[4]) << 8) | static_cast<uint8_t>(response[5]);
    
    return (address == expectedAddress) && (value == expectedValue);
}

bool ModbusFrame::parseWriteSingleCoilResponse(const QByteArray& response,
                                               uint16_t expectedAddress,
                                               bool expectedValue) {
    if (response.size() != 8) {
        return false;
    }

    if (!verifyCRC(response)) {
        return false;
    }

    if (static_cast<uint8_t>(response[1]) != static_cast<uint8_t>(FunctionCode::WriteSingleCoil)) {
        return false;
    }

    const uint16_t address = (static_cast<uint8_t>(response[2]) << 8) | static_cast<uint8_t>(response[3]);
    const uint16_t value = (static_cast<uint8_t>(response[4]) << 8) | static_cast<uint8_t>(response[5]);

    return address == expectedAddress && value == (expectedValue ? 0xFF00 : 0x0000);
}

uint16_t ModbusFrame::calculateCRC16(const QByteArray& data) {
    uint16_t crc = 0xFFFF;
    
    for (int i = 0; i < data.size(); ++i) {
        crc ^= static_cast<uint8_t>(data[i]);
        
        for (int j = 0; j < 8; ++j) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    
    return crc;
}

bool ModbusFrame::verifyCRC(const QByteArray& frame) {
    if (frame.size() < 4) { // Minimum: SlaveId + FunctionCode + CRC
        return false;
    }
    
    // Extract received CRC (last 2 bytes, little-endian)
    int crcOffset = frame.size() - 2;
    uint16_t receivedCRC = static_cast<uint8_t>(frame[crcOffset]) | 
                           (static_cast<uint8_t>(frame[crcOffset + 1]) << 8);
    
    // Calculate CRC for frame without CRC bytes
    QByteArray dataWithoutCRC = frame.left(frame.size() - 2);
    uint16_t calculatedCRC = calculateCRC16(dataWithoutCRC);
    
    return receivedCRC == calculatedCRC;
}

} // namespace Bus
} // namespace Infrastructure
