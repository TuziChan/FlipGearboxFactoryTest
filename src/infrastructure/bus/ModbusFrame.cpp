#include "ModbusFrame.h"

namespace Infrastructure {
namespace Bus {

bool ModbusFrame::s_crcBigEndian = false;

QByteArray ModbusFrame::buildReadCoils(uint8_t slaveId, uint16_t startAddress, uint16_t count) {
    QByteArray frame;
    frame.append(static_cast<char>(slaveId));
    frame.append(static_cast<char>(FunctionCode::ReadCoils));
    frame.append(static_cast<char>((startAddress >> 8) & 0xFF));
    frame.append(static_cast<char>(startAddress & 0xFF));
    frame.append(static_cast<char>((count >> 8) & 0xFF));
    frame.append(static_cast<char>(count & 0xFF));

    const uint16_t crc = calculateCRC16(frame);
    if (s_crcBigEndian) {
        frame.append(static_cast<char>((crc >> 8) & 0xFF));
        frame.append(static_cast<char>(crc & 0xFF));
    } else {
        frame.append(static_cast<char>(crc & 0xFF));
        frame.append(static_cast<char>((crc >> 8) & 0xFF));
    }

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
    if (s_crcBigEndian) {
        frame.append(static_cast<char>((crc >> 8) & 0xFF));
        frame.append(static_cast<char>(crc & 0xFF));
    } else {
        frame.append(static_cast<char>(crc & 0xFF));
        frame.append(static_cast<char>((crc >> 8) & 0xFF));
    }
    
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
    if (s_crcBigEndian) {
        frame.append(static_cast<char>((crc >> 8) & 0xFF));
        frame.append(static_cast<char>(crc & 0xFF));
    } else {
        frame.append(static_cast<char>(crc & 0xFF));
        frame.append(static_cast<char>((crc >> 8) & 0xFF));
    }

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
    if (s_crcBigEndian) {
        frame.append(static_cast<char>((crc >> 8) & 0xFF));
        frame.append(static_cast<char>(crc & 0xFF));
    } else {
        frame.append(static_cast<char>(crc & 0xFF));
        frame.append(static_cast<char>((crc >> 8) & 0xFF));
    }

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
    if (s_crcBigEndian) {
        frame.append(static_cast<char>((crc >> 8) & 0xFF));
        frame.append(static_cast<char>(crc & 0xFF));
    } else {
        frame.append(static_cast<char>(crc & 0xFF));
        frame.append(static_cast<char>((crc >> 8) & 0xFF));
    }
    
    return frame;
}

QByteArray ModbusFrame::buildWriteSingleRegisterSigned(uint8_t slaveId, uint16_t registerAddress, int16_t value) {
    // Reinterpret signed as unsigned for transmission
    uint16_t unsignedValue = static_cast<uint16_t>(value);
    return buildWriteSingleRegister(slaveId, registerAddress, unsignedValue);
}

QByteArray ModbusFrame::buildWriteMultipleRegisters(uint8_t slaveId, uint16_t startAddress, const QVector<uint16_t>& values) {
    QByteArray frame;
    frame.append(static_cast<char>(slaveId));
    frame.append(static_cast<char>(FunctionCode::WriteMultipleRegisters));
    frame.append(static_cast<char>((startAddress >> 8) & 0xFF));
    frame.append(static_cast<char>(startAddress & 0xFF));
    
    uint16_t count = static_cast<uint16_t>(values.size());
    frame.append(static_cast<char>((count >> 8) & 0xFF));
    frame.append(static_cast<char>(count & 0xFF));
    
    uint8_t byteCount = static_cast<uint8_t>(count * 2);
    frame.append(static_cast<char>(byteCount));
    
    for (uint16_t value : values) {
        frame.append(static_cast<char>((value >> 8) & 0xFF));
        frame.append(static_cast<char>(value & 0xFF));
    }
    
    uint16_t crc = calculateCRC16(frame);
    if (s_crcBigEndian) {
        frame.append(static_cast<char>((crc >> 8) & 0xFF));
        frame.append(static_cast<char>(crc & 0xFF));
    } else {
        frame.append(static_cast<char>(crc & 0xFF));
        frame.append(static_cast<char>((crc >> 8) & 0xFF));
    }
    
    return frame;
}

QByteArray ModbusFrame::buildReadDeviceIdentification(uint8_t slaveId) {
    QByteArray frame;
    frame.append(static_cast<char>(slaveId));
    frame.append(static_cast<char>(FunctionCode::ReadDeviceIdentification));
    frame.append(static_cast<char>(0x0E)); // MEI Type
    frame.append(static_cast<char>(0x01)); // Read Device ID code (basic device identification)
    frame.append(static_cast<char>(0x00)); // Object ID (start from VendorName)
    
    uint16_t crc = calculateCRC16(frame);
    if (s_crcBigEndian) {
        frame.append(static_cast<char>((crc >> 8) & 0xFF));
        frame.append(static_cast<char>(crc & 0xFF));
    } else {
        frame.append(static_cast<char>(crc & 0xFF));
        frame.append(static_cast<char>((crc >> 8) & 0xFF));
    }
    
    return frame;
}

int ModbusFrame::tryGetExpectedResponseLength(const QByteArray& request,
                                              const QByteArray& responsePrefix) {
    if (responsePrefix.size() < 2) {
        return -1;
    }

    const uint8_t functionCode = static_cast<uint8_t>(responsePrefix[1]);
    if (functionCode & 0x80) {
        return 5;
    }

    switch (functionCode) {
    case static_cast<uint8_t>(FunctionCode::ReadCoils):
    case static_cast<uint8_t>(FunctionCode::ReadHoldingRegisters):
    case static_cast<uint8_t>(FunctionCode::ReadInputRegisters):
        if (responsePrefix.size() < 3) {
            return -1;
        }
        return 3 + static_cast<uint8_t>(responsePrefix[2]) + 2;

    case static_cast<uint8_t>(FunctionCode::WriteSingleCoil):
    case static_cast<uint8_t>(FunctionCode::WriteSingleRegister):
    case static_cast<uint8_t>(FunctionCode::WriteMultipleRegisters):
        return 8;

    case static_cast<uint8_t>(FunctionCode::ReadDeviceIdentification): {
        if (responsePrefix.size() < 8) {
            return -1;
        }

        const int numObjects = static_cast<uint8_t>(responsePrefix[7]);
        int offset = 8;
        for (int i = 0; i < numObjects; ++i) {
            if (responsePrefix.size() < offset + 2) {
                return -1;
            }

            const int objectLength = static_cast<uint8_t>(responsePrefix[offset + 1]);
            offset += 2;
            if (responsePrefix.size() < offset + objectLength) {
                return -1;
            }
            offset += objectLength;
        }

        return offset + 2;
    }

    default:
        break;
    }

    if (request.size() >= 2) {
        const uint8_t requestFunctionCode = static_cast<uint8_t>(request[1]);
        switch (requestFunctionCode) {
        case static_cast<uint8_t>(FunctionCode::WriteSingleCoil):
        case static_cast<uint8_t>(FunctionCode::WriteSingleRegister):
        case static_cast<uint8_t>(FunctionCode::WriteMultipleRegisters):
            return 8;
        default:
            break;
        }
    }

    return -1;
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
    // Check for exception response
    if (functionCode & 0x80) {
        return false;
    }
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

    const uint8_t functionCode = static_cast<uint8_t>(response[1]);
    // Check for exception response
    if (functionCode & 0x80) {
        return false;
    }
    if (functionCode != static_cast<uint8_t>(FunctionCode::ReadInputRegisters)) {
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

    const uint8_t functionCode = static_cast<uint8_t>(response[1]);
    // Check for exception response
    if (functionCode & 0x80) {
        return false;
    }
    if (functionCode != static_cast<uint8_t>(FunctionCode::ReadCoils)) {
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
    // Check for exception response
    if (functionCode & 0x80) {
        return false;
    }
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

    const uint8_t functionCode = static_cast<uint8_t>(response[1]);
    // Check for exception response
    if (functionCode & 0x80) {
        return false;
    }
    if (functionCode != static_cast<uint8_t>(FunctionCode::WriteSingleCoil)) {
        return false;
    }

    const uint16_t address = (static_cast<uint8_t>(response[2]) << 8) | static_cast<uint8_t>(response[3]);
    const uint16_t value = (static_cast<uint8_t>(response[4]) << 8) | static_cast<uint8_t>(response[5]);

    return address == expectedAddress && value == (expectedValue ? 0xFF00 : 0x0000);
}

bool ModbusFrame::parseWriteMultipleRegistersResponse(const QByteArray& response,
                                                       uint16_t expectedAddress,
                                                       uint16_t expectedCount) {
    // Response format: SlaveId(1) + FunctionCode(1) + Address(2) + Count(2) + CRC(2) = 8 bytes
    if (response.size() != 8) {
        return false;
    }
    
    if (!verifyCRC(response)) {
        return false;
    }
    
    uint8_t functionCode = static_cast<uint8_t>(response[1]);
    // Check for exception response
    if (functionCode & 0x80) {
        return false;
    }
    if (functionCode != static_cast<uint8_t>(FunctionCode::WriteMultipleRegisters)) {
        return false;
    }
    
    uint16_t address = (static_cast<uint8_t>(response[2]) << 8) | static_cast<uint8_t>(response[3]);
    uint16_t count = (static_cast<uint8_t>(response[4]) << 8) | static_cast<uint8_t>(response[5]);
    
    return (address == expectedAddress) && (count == expectedCount);
}

bool ModbusFrame::parseReadDeviceIdentificationResponse(const QByteArray& response,
                                                         QString& outVendor,
                                                         QString& outProduct,
                                                         QString& outVersion) {
    // Minimum response: SlaveId(1) + FunctionCode(1) + MEI(1) + ReadDevID(1) + Conformity(1) + MoreFollows(1) + NextObjectId(1) + NumObjects(1) + CRC(2) = 10 bytes
    if (response.size() < 10) {
        return false;
    }
    
    if (!verifyCRC(response)) {
        return false;
    }
    
    uint8_t functionCode = static_cast<uint8_t>(response[1]);
    // Check for exception response
    if (functionCode & 0x80) {
        return false;
    }
    if (functionCode != static_cast<uint8_t>(FunctionCode::ReadDeviceIdentification)) {
        return false;
    }
    
    uint8_t meiType = static_cast<uint8_t>(response[2]);
    if (meiType != 0x0E) {
        return false;
    }
    
    // Skip ReadDevID(1), Conformity(1), MoreFollows(1), NextObjectId(1)
    uint8_t numObjects = static_cast<uint8_t>(response[7]);
    
    int offset = 8;
    for (int i = 0; i < numObjects && offset < response.size() - 2; ++i) {
        if (offset + 2 > response.size() - 2) break;
        
        uint8_t objectId = static_cast<uint8_t>(response[offset]);
        uint8_t objectLength = static_cast<uint8_t>(response[offset + 1]);
        offset += 2;
        
        if (offset + objectLength > response.size() - 2) break;
        
        QString objectValue = QString::fromLatin1(response.mid(offset, objectLength));
        offset += objectLength;
        
        switch (objectId) {
            case 0x00: // VendorName
                outVendor = objectValue;
                break;
            case 0x01: // ProductCode
                outProduct = objectValue;
                break;
            case 0x02: // MajorMinorRevision
                outVersion = objectValue;
                break;
            default:
                break;
        }
    }
    
    return true;
}

QPair<uint8_t, QString> ModbusFrame::parseExceptionResponse(const QByteArray& response) {
    if (response.size() < 5) { // SlaveId + FunctionCode + ExceptionCode + CRC(2)
        return QPair<uint8_t, QString>(0, "Invalid exception response length");
    }
    
    if (!verifyCRC(response)) {
        return QPair<uint8_t, QString>(0, "CRC verification failed");
    }
    
    uint8_t functionCode = static_cast<uint8_t>(response[1]);
    if (!(functionCode & 0x80)) {
        return QPair<uint8_t, QString>(0, "Not an exception response");
    }
    
    uint8_t exceptionCode = static_cast<uint8_t>(response[2]);
    QString description = exceptionCodeToString(exceptionCode);
    
    return QPair<uint8_t, QString>(exceptionCode, description);
}

QString ModbusFrame::exceptionCodeToString(uint8_t code) {
    switch (code) {
        case 0x01:
            return "Illegal Function (0x01)";
        case 0x02:
            return "Illegal Data Address (0x02)";
        case 0x03:
            return "Illegal Data Value (0x03)";
        case 0x04:
            return "Slave Device Failure (0x04)";
        case 0x05:
            return "Acknowledge (0x05)";
        case 0x06:
            return "Slave Device Busy (0x06)";
        case 0x07:
            return "Negative Acknowledge (0x07)";
        case 0x08:
            return "Memory Parity Error (0x08)";
        case 0x0A:
            return "Gateway Path Unavailable (0x0A)";
        case 0x0B:
            return "Gateway Target Device Failed to Respond (0x0B)";
        case 0x40:
            return "AQMD: Operation Forbidden (0x40)";
        case 0xFF:
            return "AQMD: Undefined Error (0xFF)";
        default:
            return QString("Unknown Exception Code (0x%1)").arg(code, 2, 16, QChar('0'));
    }
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
    
    // Extract received CRC
    int crcOffset = frame.size() - 2;
    uint16_t receivedCRC;
    if (s_crcBigEndian) {
        receivedCRC = (static_cast<uint8_t>(frame[crcOffset]) << 8) | 
                      static_cast<uint8_t>(frame[crcOffset + 1]);
    } else {
        receivedCRC = static_cast<uint8_t>(frame[crcOffset]) | 
                      (static_cast<uint8_t>(frame[crcOffset + 1]) << 8);
    }
    
    // Calculate CRC for frame without CRC bytes
    QByteArray dataWithoutCRC = frame.left(frame.size() - 2);
    uint16_t calculatedCRC = calculateCRC16(dataWithoutCRC);
    
    return receivedCRC == calculatedCRC;
}

void ModbusFrame::setCrcByteOrder(bool bigEndian) {
    s_crcBigEndian = bigEndian;
}

bool ModbusFrame::isCrcBigEndian() {
    return s_crcBigEndian;
}

} // namespace Bus
} // namespace Infrastructure
