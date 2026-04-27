#include <QTest>
#include <QVector>
#include "../../src/infrastructure/bus/ModbusFrame.h"

using namespace Infrastructure::Bus;

class ModbusCrcTests : public QObject {
    Q_OBJECT

private slots:
    void testCrcCalculation();
    void testCrcValidation();
    void testReadRequest();
    void testWriteRequest();
    void testReadResponse();
    void testInvalidFrame();
};

void ModbusCrcTests::testCrcCalculation() {
    // Build a Read Holding Registers (0x03) request frame using static API
    QByteArray frame = ModbusFrame::buildReadHoldingRegisters(1, 0x0000, 2);

    // Verify it matches the known-good frame
    QCOMPARE(frame, QByteArray::fromHex("010300000002C40B"));

    QVERIFY(frame.size() >= 2);

    // Extract CRC from last 2 bytes (little-endian)
    uint16_t frameCrc = static_cast<uint8_t>(frame[frame.size() - 2]) |
                        (static_cast<uint8_t>(frame[frame.size() - 1]) << 8);

    // Calculate CRC for the data portion (frame without CRC bytes)
    uint16_t calculatedCrc = ModbusFrame::calculateCRC16(frame.left(frame.size() - 2));

    QCOMPARE(frameCrc, calculatedCrc);
    QVERIFY(frameCrc != 0);
}

void ModbusCrcTests::testCrcValidation() {
    QByteArray validFrame = QByteArray::fromHex("010300000002C40B");

    // Verify CRC using static API
    QVERIFY(ModbusFrame::verifyCRC(validFrame));

    // Also verify the frame matches what buildReadHoldingRegisters produces
    QByteArray builtFrame = ModbusFrame::buildReadHoldingRegisters(1, 0x0000, 2);
    QCOMPARE(validFrame, builtFrame);
}

void ModbusCrcTests::testReadRequest() {
    // Build a Read Holding Registers request: slave=2, address=0x0010, count=1
    QByteArray frame = ModbusFrame::buildReadHoldingRegisters(2, 0x0010, 1);

    QCOMPARE(frame.size(), 8); // SlaveId(1) + Func(1) + Addr(2) + Count(2) + CRC(2)
    QCOMPARE(static_cast<uint8_t>(frame[0]), static_cast<uint8_t>(2));
    QCOMPARE(static_cast<uint8_t>(frame[1]), static_cast<uint8_t>(ModbusFrame::FunctionCode::ReadHoldingRegisters));
    QCOMPARE(static_cast<uint8_t>(frame[2]), static_cast<uint8_t>(0x00));
    QCOMPARE(static_cast<uint8_t>(frame[3]), static_cast<uint8_t>(0x10));
    QCOMPARE(static_cast<uint8_t>(frame[4]), static_cast<uint8_t>(0x00));
    QCOMPARE(static_cast<uint8_t>(frame[5]), static_cast<uint8_t>(0x01));

    QVERIFY(ModbusFrame::verifyCRC(frame));
}

void ModbusCrcTests::testWriteRequest() {
    // Build a Write Single Register request: slave=1, address=0x0020, value=0x1234
    QByteArray frame = ModbusFrame::buildWriteSingleRegister(1, 0x0020, 0x1234);

    QCOMPARE(frame.size(), 8);
    QCOMPARE(static_cast<uint8_t>(frame[0]), static_cast<uint8_t>(1));
    QCOMPARE(static_cast<uint8_t>(frame[1]), static_cast<uint8_t>(ModbusFrame::FunctionCode::WriteSingleRegister));
    QCOMPARE(static_cast<uint8_t>(frame[2]), static_cast<uint8_t>(0x00));
    QCOMPARE(static_cast<uint8_t>(frame[3]), static_cast<uint8_t>(0x20));
    QCOMPARE(static_cast<uint8_t>(frame[4]), static_cast<uint8_t>(0x12));
    QCOMPARE(static_cast<uint8_t>(frame[5]), static_cast<uint8_t>(0x34));

    QVERIFY(ModbusFrame::verifyCRC(frame));
}

void ModbusCrcTests::testReadResponse() {
    // Build a valid Read Holding Registers response frame manually
    QByteArray response;
    response.append(static_cast<char>(0x01)); // slaveId
    response.append(static_cast<char>(0x03)); // functionCode
    response.append(static_cast<char>(0x04)); // byteCount = 4 bytes (2 registers)
    response.append(static_cast<char>(0x12));
    response.append(static_cast<char>(0x34));
    response.append(static_cast<char>(0x56));
    response.append(static_cast<char>(0x78));

    // Calculate CRC for the data portion and append it
    uint16_t crc = ModbusFrame::calculateCRC16(response);
    response.append(static_cast<char>(crc & 0xFF));
    response.append(static_cast<char>((crc >> 8) & 0xFF));

    // Verify CRC of the complete frame
    QVERIFY(ModbusFrame::verifyCRC(response));

    // Parse the response and verify register values
    QVector<uint16_t> values;
    QVERIFY(ModbusFrame::parseReadHoldingRegistersResponse(response, 2, values));
    QCOMPARE(values.size(), 2);
    QCOMPARE(values[0], static_cast<uint16_t>(0x1234));
    QCOMPARE(values[1], static_cast<uint16_t>(0x5678));
}

void ModbusCrcTests::testInvalidFrame() {
    QByteArray invalidFrame = QByteArray::fromHex("010300000002FFFF");

    // verifyCRC should reject a frame with incorrect CRC
    QVERIFY(!ModbusFrame::verifyCRC(invalidFrame));
}

QTEST_MAIN(ModbusCrcTests)
#include "ModbusCrcTests.moc"
