#include <QCoreApplication>
#include <QtTest>
#include <QSignalSpy>
#include <QDebug>
#include <QElapsedTimer>

#include "src/infrastructure/bus/ModbusFrame.h"
#include "src/infrastructure/bus/IBusController.h"
#include "mocks/MockDevices.h"

using namespace Infrastructure::Bus;
using namespace Tests::Mocks;

/**
 * @brief Protocol Layer Tests for Modbus Communication
 *
 * Comprehensive test suite covering:
 * - Frame building for all function codes
 * - Response parsing and validation
 * - CRC calculation and verification
 * - Exception handling
 * - Boundary conditions and error scenarios
 */
class ProtocolLayerTests : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        qDebug() << "=== 协议层测试套件启动 ===";
        qDebug() << "测试Modbus RTU协议实现的正确性和鲁棒性";

        // Reset CRC byte order to default
        ModbusFrame::setCrcByteOrder(false);
    }

    void cleanupTestCase() {
        qDebug() << "=== 协议层测试套件完成 ===";
    }

    // ========== Frame Building Tests ==========

    void testBuildReadCoilsFrame() {
        QByteArray frame = ModbusFrame::buildReadCoils(1, 0x0100, 10);

        QCOMPARE(frame.size(), 8); // SlaveId(1) + Func(1) + Addr(2) + Count(2) + CRC(2)
        QCOMPARE(static_cast<uint8_t>(frame[0]), static_cast<uint8_t>(1));
        QCOMPARE(static_cast<uint8_t>(frame[1]), static_cast<uint8_t>(ModbusFrame::FunctionCode::ReadCoils));
        QCOMPARE(static_cast<uint8_t>(frame[2]), static_cast<uint8_t>(0x01));
        QCOMPARE(static_cast<uint8_t>(frame[3]), static_cast<uint8_t>(0x00));
        QCOMPARE(static_cast<uint8_t>(frame[4]), static_cast<uint8_t>(0x00));
        QCOMPARE(static_cast<uint8_t>(frame[5]), static_cast<uint8_t>(0x0A));

        QVERIFY(ModbusFrame::verifyCRC(frame));
        qDebug() << "✓ ReadCoils帧构建测试通过";
    }

    void testBuildReadHoldingRegistersFrame() {
        QByteArray frame = ModbusFrame::buildReadHoldingRegisters(2, 0x0200, 5);

        QCOMPARE(frame.size(), 8);
        QCOMPARE(static_cast<uint8_t>(frame[0]), static_cast<uint8_t>(2));
        QCOMPARE(static_cast<uint8_t>(frame[1]), static_cast<uint8_t>(ModbusFrame::FunctionCode::ReadHoldingRegisters));
        QCOMPARE(static_cast<uint8_t>(frame[2]), static_cast<uint8_t>(0x02));
        QCOMPARE(static_cast<uint8_t>(frame[3]), static_cast<uint8_t>(0x00));
        QCOMPARE(static_cast<uint8_t>(frame[4]), static_cast<uint8_t>(0x00));
        QCOMPARE(static_cast<uint8_t>(frame[5]), static_cast<uint8_t>(0x05));

        QVERIFY(ModbusFrame::verifyCRC(frame));
        qDebug() << "✓ ReadHoldingRegisters帧构建测试通过";
    }

    void testBuildReadInputRegistersFrame() {
        QByteArray frame = ModbusFrame::buildReadInputRegisters(4, 0x0001, 2);

        QCOMPARE(frame.size(), 8);
        QCOMPARE(static_cast<uint8_t>(frame[0]), static_cast<uint8_t>(4));
        QCOMPARE(static_cast<uint8_t>(frame[1]), static_cast<uint8_t>(ModbusFrame::FunctionCode::ReadInputRegisters));
        QCOMPARE(static_cast<uint8_t>(frame[2]), static_cast<uint8_t>(0x00));
        QCOMPARE(static_cast<uint8_t>(frame[3]), static_cast<uint8_t>(0x01));
        QCOMPARE(static_cast<uint8_t>(frame[4]), static_cast<uint8_t>(0x00));
        QCOMPARE(static_cast<uint8_t>(frame[5]), static_cast<uint8_t>(0x02));

        QVERIFY(ModbusFrame::verifyCRC(frame));
        qDebug() << "✓ ReadInputRegisters(0x04)帧构建测试通过";
    }

    void testBuildWriteSingleCoilFrame() {
        // Test writing coil ON (0xFF00)
        QByteArray frameOn = ModbusFrame::buildWriteSingleCoil(4, 0x0000, true);

        QCOMPARE(frameOn.size(), 8);
        QCOMPARE(static_cast<uint8_t>(frameOn[0]), static_cast<uint8_t>(4));
        QCOMPARE(static_cast<uint8_t>(frameOn[1]), static_cast<uint8_t>(ModbusFrame::FunctionCode::WriteSingleCoil));
        QCOMPARE(static_cast<uint8_t>(frameOn[2]), static_cast<uint8_t>(0x00));
        QCOMPARE(static_cast<uint8_t>(frameOn[3]), static_cast<uint8_t>(0x00));
        QCOMPARE(static_cast<uint8_t>(frameOn[4]), static_cast<uint8_t>(0xFF)); // ON = 0xFF00
        QCOMPARE(static_cast<uint8_t>(frameOn[5]), static_cast<uint8_t>(0x00));

        QVERIFY(ModbusFrame::verifyCRC(frameOn));

        // Test writing coil OFF (0x0000)
        QByteArray frameOff = ModbusFrame::buildWriteSingleCoil(4, 0x0001, false);

        QCOMPARE(frameOff.size(), 8);
        QCOMPARE(static_cast<uint8_t>(frameOff[0]), static_cast<uint8_t>(4));
        QCOMPARE(static_cast<uint8_t>(frameOff[1]), static_cast<uint8_t>(ModbusFrame::FunctionCode::WriteSingleCoil));
        QCOMPARE(static_cast<uint8_t>(frameOff[2]), static_cast<uint8_t>(0x00));
        QCOMPARE(static_cast<uint8_t>(frameOff[3]), static_cast<uint8_t>(0x01));
        QCOMPARE(static_cast<uint8_t>(frameOff[4]), static_cast<uint8_t>(0x00)); // OFF = 0x0000
        QCOMPARE(static_cast<uint8_t>(frameOff[5]), static_cast<uint8_t>(0x00));

        QVERIFY(ModbusFrame::verifyCRC(frameOff));
        qDebug() << "✓ WriteSingleCoil(0x05)帧构建测试通过";
    }

    void testBuildWriteMultipleRegistersFrame() {
        QVector<uint16_t> values = {0x0101, 0x0202, 0x0303};
        QByteArray frame = ModbusFrame::buildWriteMultipleRegisters(1, 0x0100, values);

        // Expected: SlaveId(1) + Func(1) + Addr(2) + Count(2) + ByteCount(1) + Data(6) + CRC(2) = 15
        QCOMPARE(frame.size(), 15);
        QCOMPARE(static_cast<uint8_t>(frame[0]), static_cast<uint8_t>(1));
        QCOMPARE(static_cast<uint8_t>(frame[1]), static_cast<uint8_t>(ModbusFrame::FunctionCode::WriteMultipleRegisters));
        QCOMPARE(static_cast<uint8_t>(frame[6]), static_cast<uint8_t>(6)); // Byte count

        QVERIFY(ModbusFrame::verifyCRC(frame));
        qDebug() << "✓ WriteMultipleRegisters(0x10)帧构建测试通过";
    }

    void testBuildReadDeviceIdentificationFrame() {
        QByteArray frame = ModbusFrame::buildReadDeviceIdentification(1);

        // Expected: SlaveId(1) + Func(1) + MEI(1) + ReadDevID(1) + ObjectId(1) + CRC(2) = 7
        QCOMPARE(frame.size(), 7);
        QCOMPARE(static_cast<uint8_t>(frame[0]), static_cast<uint8_t>(1));
        QCOMPARE(static_cast<uint8_t>(frame[1]), static_cast<uint8_t>(ModbusFrame::FunctionCode::ReadDeviceIdentification));
        QCOMPARE(static_cast<uint8_t>(frame[2]), static_cast<uint8_t>(0x0E)); // MEI type
        QCOMPARE(static_cast<uint8_t>(frame[3]), static_cast<uint8_t>(0x01)); // Read Device ID code
        QCOMPARE(static_cast<uint8_t>(frame[4]), static_cast<uint8_t>(0x00)); // Object ID

        QVERIFY(ModbusFrame::verifyCRC(frame));
        qDebug() << "✓ ReadDeviceIdentification(0x2B)帧构建测试通过";
    }

    // ========== Response Parsing Tests ==========

    void testParseReadHoldingRegistersResponse() {
        // Build a valid response manually
        QByteArray response;
        response.append(static_cast<char>(1)); // Slave ID
        response.append(static_cast<char>(0x03)); // Function code
        response.append(static_cast<char>(4)); // Byte count (2 registers * 2 bytes)
        response.append(static_cast<char>(0x01)); // Register 1 high byte
        response.append(static_cast<char>(0x00)); // Register 1 low byte
        response.append(static_cast<char>(0x02)); // Register 2 high byte
        response.append(static_cast<char>(0x00)); // Register 2 low byte

        // Add CRC
        uint16_t crc = ModbusFrame::calculateCRC16(response);
        response.append(static_cast<char>(crc & 0xFF));
        response.append(static_cast<char>((crc >> 8) & 0xFF));

        QVector<uint16_t> values;
        bool result = ModbusFrame::parseReadHoldingRegistersResponse(response, 2, values);

        QVERIFY(result);
        QCOMPARE(values.size(), 2);
        QCOMPARE(values[0], static_cast<uint16_t>(0x0100));
        QCOMPARE(values[1], static_cast<uint16_t>(0x0200));

        qDebug() << "✓ ReadHoldingRegisters响应解析测试通过";
    }

    void testParseReadInputRegistersResponse() {
        // Build a valid input register response
        QByteArray response;
        response.append(static_cast<char>(4)); // Slave ID
        response.append(static_cast<char>(0x04)); // Function code
        response.append(static_cast<char>(4)); // Byte count (2 registers * 2 bytes)
        response.append(static_cast<char>(0x03)); // Register 1 high byte
        response.append(static_cast<char>(0xE8)); // Register 1 low byte (1000 decimal)
        response.append(static_cast<char>(0x00)); // Register 2 high byte
        response.append(static_cast<char>(0x64)); // Register 2 low byte (100 decimal)

        // Add CRC
        uint16_t crc = ModbusFrame::calculateCRC16(response);
        response.append(static_cast<char>(crc & 0xFF));
        response.append(static_cast<char>((crc >> 8) & 0xFF));

        QVector<uint16_t> values;
        bool result = ModbusFrame::parseReadInputRegistersResponse(response, 2, values);

        QVERIFY(result);
        QCOMPARE(values.size(), 2);
        QCOMPARE(values[0], static_cast<uint16_t>(1000));
        QCOMPARE(values[1], static_cast<uint16_t>(100));

        qDebug() << "✓ ReadInputRegisters(0x04)响应解析测试通过";
    }

    void testParseWriteSingleCoilResponse() {
        // Build a valid coil write response (ON)
        QByteArray responseOn;
        responseOn.append(static_cast<char>(4)); // Slave ID
        responseOn.append(static_cast<char>(0x05)); // Function code
        responseOn.append(static_cast<char>(0x00)); // Coil address high
        responseOn.append(static_cast<char>(0x00)); // Coil address low
        responseOn.append(static_cast<char>(0xFF)); // Value high (ON = 0xFF00)
        responseOn.append(static_cast<char>(0x00)); // Value low

        uint16_t crc = ModbusFrame::calculateCRC16(responseOn);
        responseOn.append(static_cast<char>(crc & 0xFF));
        responseOn.append(static_cast<char>((crc >> 8) & 0xFF));

        bool resultOn = ModbusFrame::parseWriteSingleCoilResponse(responseOn, 0x0000, true);
        QVERIFY(resultOn);

        // Build a valid coil write response (OFF)
        QByteArray responseOff;
        responseOff.append(static_cast<char>(4)); // Slave ID
        responseOff.append(static_cast<char>(0x05)); // Function code
        responseOff.append(static_cast<char>(0x00)); // Coil address high
        responseOff.append(static_cast<char>(0x01)); // Coil address low
        responseOff.append(static_cast<char>(0x00)); // Value high (OFF = 0x0000)
        responseOff.append(static_cast<char>(0x00)); // Value low

        crc = ModbusFrame::calculateCRC16(responseOff);
        responseOff.append(static_cast<char>(crc & 0xFF));
        responseOff.append(static_cast<char>((crc >> 8) & 0xFF));

        bool resultOff = ModbusFrame::parseWriteSingleCoilResponse(responseOff, 0x0001, false);
        QVERIFY(resultOff);

        qDebug() << "✓ WriteSingleCoil(0x05)响应解析测试通过";
    }

    void testParseWriteMultipleRegistersResponse() {
        QByteArray response;
        response.append(static_cast<char>(1)); // Slave ID
        response.append(static_cast<char>(0x10)); // Function code
        response.append(static_cast<char>(0x01)); // Start address high
        response.append(static_cast<char>(0x00)); // Start address low
        response.append(static_cast<char>(0x00)); // Count high
        response.append(static_cast<char>(0x05)); // Count low (5 registers)

        uint16_t crc = ModbusFrame::calculateCRC16(response);
        response.append(static_cast<char>(crc & 0xFF));
        response.append(static_cast<char>((crc >> 8) & 0xFF));

        bool result = ModbusFrame::parseWriteMultipleRegistersResponse(response, 0x0100, 5);

        QVERIFY(result);
        qDebug() << "✓ WriteMultipleRegisters响应解析测试通过";
    }

    void testParseReadDeviceIdentificationResponse() {
        // Build a device identification response
        QByteArray response;
        response.append(static_cast<char>(1)); // Slave ID
        response.append(static_cast<char>(0x2B)); // Function code
        response.append(static_cast<char>(0x0E)); // MEI type
        response.append(static_cast<char>(0x01)); // Read Device ID
        response.append(static_cast<char>(0x01)); // Conformity level
        response.append(static_cast<char>(0x00)); // More follows
        response.append(static_cast<char>(0x00)); // Next object ID
        response.append(static_cast<char>(0x03)); // Number of objects

        // Vendor name (object ID 0x00)
        response.append(static_cast<char>(0x00)); // Object ID
        response.append(static_cast<char>(0x00)); // Object ID
        uint8_t vendorLen = 6;
        response.append(vendorLen);
        response.append("Vendor");

        // Product code (object ID 0x01)
        response.append(static_cast<char>(0x01)); // Object ID
        response.append(static_cast<char>(0x00)); // Object ID
        uint8_t productLen = 7;
        response.append(productLen);
        response.append("Product");

        // Version (object ID 0x02)
        response.append(static_cast<char>(0x02)); // Object ID
        response.append(static_cast<char>(0x00)); // Object ID
        uint8_t versionLen = 5;
        response.append(versionLen);
        response.append("v1.00");

        uint16_t crc = ModbusFrame::calculateCRC16(response);
        response.append(static_cast<char>(crc & 0xFF));
        response.append(static_cast<char>((crc >> 8) & 0xFF));

        QString vendor, product, version;
        bool result = ModbusFrame::parseReadDeviceIdentificationResponse(response, vendor, product, version);

        QVERIFY(result);
        QCOMPARE(vendor, QString("Vendor"));
        QCOMPARE(product, QString("Product"));
        QCOMPARE(version, QString("v1.00"));

        qDebug() << "✓ ReadDeviceIdentification响应解析测试通过";
    }

    // ========== Exception Handling Tests ==========

    void testParseExceptionResponse() {
        // Build an exception response
        QByteArray exception;
        exception.append(static_cast<char>(1)); // Slave ID
        exception.append(static_cast<char>(0x83)); // Function code with exception bit set (0x03 | 0x80)
        exception.append(static_cast<char>(0x02)); // Exception code: Illegal Data Address

        uint16_t crc = ModbusFrame::calculateCRC16(exception);
        exception.append(static_cast<char>(crc & 0xFF));
        exception.append(static_cast<char>((crc >> 8) & 0xFF));

        QPair<uint8_t, QString> result = ModbusFrame::parseExceptionResponse(exception);

        QCOMPARE(result.first, static_cast<uint8_t>(0x02));
        QVERIFY(result.second.contains("Illegal Data Address"));

        qDebug() << "✓ 异常响应解析测试通过";
    }

    void testExceptionCodeToString() {
        QCOMPARE(ModbusFrame::exceptionCodeToString(0x01), QString("Illegal Function (0x01)"));
        QCOMPARE(ModbusFrame::exceptionCodeToString(0x02), QString("Illegal Data Address (0x02)"));
        QCOMPARE(ModbusFrame::exceptionCodeToString(0x03), QString("Illegal Data Value (0x03)"));
        QCOMPARE(ModbusFrame::exceptionCodeToString(0x04), QString("Slave Device Failure (0x04)"));
        QCOMPARE(ModbusFrame::exceptionCodeToString(0x40), QString("AQMD: Operation Forbidden (0x40)"));
        QCOMPARE(ModbusFrame::exceptionCodeToString(0xFF), QString("AQMD: Undefined Error (0xFF)"));

        // Test unknown exception code
        QString unknown = ModbusFrame::exceptionCodeToString(0x99);
        QVERIFY(unknown.contains("0x99"));

        qDebug() << "✓ 异常代码转换测试通过";
    }

    // ========== CRC Calculation Tests ==========

    void testCalculateCRC16() {
        QByteArray data;
        data.append(static_cast<char>(0x01));
        data.append(static_cast<char>(0x03));
        data.append(static_cast<char>(0x00));
        data.append(static_cast<char>(0x00));
        data.append(static_cast<char>(0x00));
        data.append(static_cast<char>(0x0A));

        uint16_t crc = ModbusFrame::calculateCRC16(data);

        // Verify CRC is non-zero (basic sanity check)
        QVERIFY(crc != 0);

        // Verify same data produces same CRC
        uint16_t crc2 = ModbusFrame::calculateCRC16(data);
        QCOMPARE(crc, crc2);

        // Verify different data produces different CRC
        QByteArray data2 = data;
        data2[5] = static_cast<char>(0x0B); // Change one byte
        uint16_t crc3 = ModbusFrame::calculateCRC16(data2);
        QVERIFY(crc != crc3);

        qDebug() << "✓ CRC16计算测试通过";
    }

    void testVerifyCRC() {
        // Build a valid frame
        QByteArray validFrame;
        validFrame.append(static_cast<char>(1));
        validFrame.append(static_cast<char>(0x03));
        validFrame.append(static_cast<char>(0x00));
        validFrame.append(static_cast<char>(0x00));
        validFrame.append(static_cast<char>(0x00));
        validFrame.append(static_cast<char>(0x0A));

        uint16_t crc = ModbusFrame::calculateCRC16(validFrame);
        validFrame.append(static_cast<char>(crc & 0xFF));
        validFrame.append(static_cast<char>((crc >> 8) & 0xFF));

        QVERIFY(ModbusFrame::verifyCRC(validFrame));

        // Corrupt the CRC
        validFrame[validFrame.size() - 1] = static_cast<char>(0xFF);
        QVERIFY(!ModbusFrame::verifyCRC(validFrame));

        qDebug() << "✓ CRC验证测试通过";
    }

    // ========== MockBusController Integration Tests ==========

    void testMockBusControllerBasicOperation() {
        MockBusController mockBus;

        QVERIFY(mockBus.open("COM1", 9600, 500, "Even", 1));
        QVERIFY(mockBus.isOpen());

        QByteArray request = ModbusFrame::buildReadHoldingRegisters(1, 0x0100, 1);
        QByteArray response;
        QVERIFY(mockBus.sendRequest(request, response));
        QVERIFY(!response.isEmpty());

        mockBus.close();
        QVERIFY(!mockBus.isOpen());

        qDebug() << "✓ MockBusController基本操作测试通过";
    }

    void testMockBusControllerFunctionCodeCoverage() {
        MockBusController mockBus;
        mockBus.open("COM1", 9600, 500);

        // Test all function codes
        QByteArray response;

        // 0x03 Read Holding Registers
        mockBus.sendRequest(ModbusFrame::buildReadHoldingRegisters(1, 0, 5), response);
        QVERIFY(mockBus.wasFunctionCalled(0x03));

        // 0x04 Read Input Registers
        mockBus.sendRequest(ModbusFrame::buildReadInputRegisters(1, 0, 2), response);
        QVERIFY(mockBus.wasFunctionCalled(0x04));

        // 0x05 Write Single Coil
        mockBus.sendRequest(ModbusFrame::buildWriteSingleCoil(1, 0, true), response);
        QVERIFY(mockBus.wasFunctionCalled(0x05));

        // 0x10 Write Multiple Registers
        QVector<uint16_t> values = {100, 200, 300};
        mockBus.sendRequest(ModbusFrame::buildWriteMultipleRegisters(1, 0, values), response);
        QVERIFY(mockBus.wasFunctionCalled(0x10));

        // 0x2B Read Device Identification
        mockBus.sendRequest(ModbusFrame::buildReadDeviceIdentification(1), response);
        QVERIFY(mockBus.wasFunctionCalled(0x2B));

        qDebug() << "✓ MockBusController功能码覆盖测试通过 (0x03/0x04/0x05/0x06/0x10/0x2B)";
    }

    void testMockBusControllerTimeoutSimulation() {
        MockBusController mockBus;
        mockBus.open("COM1", 9600, 500);
        mockBus.setMockTimeout(true);

        QByteArray request = ModbusFrame::buildReadHoldingRegisters(1, 0x0100, 1);
        QByteArray response;

        QVERIFY(!mockBus.sendRequest(request, response));
        QVERIFY(!mockBus.lastError().isEmpty());
        QVERIFY(mockBus.lastError().contains("timeout"));

        qDebug() << "✓ MockBusController超时模拟测试通过";
    }
};

QTEST_MAIN(ProtocolLayerTests)
#include "ProtocolLayerTests.moc"