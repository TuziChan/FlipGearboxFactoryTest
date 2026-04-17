#ifndef MODBUSFRAME_H
#define MODBUSFRAME_H

#include <QByteArray>
#include <QList>
#include <cstdint>

namespace Infrastructure {
namespace Bus {

/**
 * @brief Modbus RTU frame builder and parser
 * 
 * Handles frame construction, CRC calculation, and response parsing
 * for Modbus RTU protocol.
 */
class ModbusFrame {
public:
    enum class FunctionCode : uint8_t {
        ReadCoils = 0x01,
        ReadHoldingRegisters = 0x03,
        ReadInputRegisters = 0x04,
        WriteSingleCoil = 0x05,
        WriteSingleRegister = 0x06,
        WriteMultipleRegisters = 0x10
    };
    static QByteArray buildReadCoils(uint8_t slaveId, uint16_t startAddress, uint16_t count);

    /**
     * @brief Build a Read Holding Registers (0x03) request
     * @param slaveId Modbus slave address
     * @param startAddress Starting register address
     * @param count Number of registers to read
     * @return Complete Modbus RTU frame with CRC
     */
    static QByteArray buildReadHoldingRegisters(uint8_t slaveId, uint16_t startAddress, uint16_t count);
    static QByteArray buildReadInputRegisters(uint8_t slaveId, uint16_t startAddress, uint16_t count);
    static QByteArray buildWriteSingleCoil(uint8_t slaveId, uint16_t coilAddress, bool value);

    /**
     * @brief Build a Write Single Register (0x06) request
     * @param slaveId Modbus slave address
     * @param registerAddress Register address to write
     * @param value Value to write
     * @return Complete Modbus RTU frame with CRC
     */
    static QByteArray buildWriteSingleRegister(uint8_t slaveId, uint16_t registerAddress, uint16_t value);

    /**
     * @brief Build a Write Single Register (0x06) request with signed value
     * @param slaveId Modbus slave address
     * @param registerAddress Register address to write
     * @param value Signed value to write
     * @return Complete Modbus RTU frame with CRC
     */
    static QByteArray buildWriteSingleRegisterSigned(uint8_t slaveId, uint16_t registerAddress, int16_t value);

    /**
     * @brief Parse Read Holding Registers response
     * @param response Raw response frame
     * @param expectedCount Expected number of registers
     * @param outValues Output vector for parsed register values
     * @return true if parsing succeeded and CRC is valid
     */
    static bool parseReadHoldingRegistersResponse(const QByteArray& response, 
                                                   uint16_t expectedCount,
                                                   QVector<uint16_t>& outValues);
    static bool parseReadInputRegistersResponse(const QByteArray& response,
                                                uint16_t expectedCount,
                                                QVector<uint16_t>& outValues);
    static bool parseReadCoilsResponse(const QByteArray& response,
                                       uint16_t expectedCount,
                                       QVector<bool>& outValues);

    /**
     * @brief Parse Write Single Register response
     * @param response Raw response frame
     * @param expectedAddress Expected register address
     * @param expectedValue Expected written value
     * @return true if response is valid and matches expected values
     */
    static bool parseWriteSingleRegisterResponse(const QByteArray& response,
                                                  uint16_t expectedAddress,
                                                  uint16_t expectedValue);
    static bool parseWriteSingleCoilResponse(const QByteArray& response,
                                             uint16_t expectedAddress,
                                             bool expectedValue);

    /**
     * @brief Calculate Modbus RTU CRC16
     * @param data Data to calculate CRC for
     * @return CRC16 value (little-endian)
     */
    static uint16_t calculateCRC16(const QByteArray& data);

    /**
     * @brief Verify CRC of a complete frame
     * @param frame Frame including CRC bytes
     * @return true if CRC is valid
     */
    static bool verifyCRC(const QByteArray& frame);

private:
    ModbusFrame() = delete;
};

} // namespace Bus
} // namespace Infrastructure

#endif // MODBUSFRAME_H
