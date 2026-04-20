#ifndef MOCKMODBUSDEVICE_H
#define MOCKMODBUSDEVICE_H

#include <QObject>
#include <QByteArray>
#include <QMap>
#include <QRandomGenerator>
#include "../bus/ModbusFrame.h"

namespace Infrastructure {
namespace Simulation {

/**
 * @brief Base class for mock Modbus RTU devices
 * 
 * Provides Modbus protocol handling, register storage, and error injection.
 */
class MockModbusDevice : public QObject {
    Q_OBJECT

public:
    struct ErrorInjectionConfig {
        bool enabled = false;
        double crcErrorRate = 0.0;      // 0.0-1.0, probability of CRC error
        double timeoutRate = 0.0;       // 0.0-1.0, probability of no response
        double delayRate = 0.0;         // 0.0-1.0, probability of delayed response
        int minDelayMs = 100;           // Minimum delay when delayRate triggers
        int maxDelayMs = 500;           // Maximum delay when delayRate triggers
        double exceptionRate = 0.0;     // 0.0-1.0, probability of exception response
    };

    explicit MockModbusDevice(uint8_t slaveId, QObject* parent = nullptr);
    virtual ~MockModbusDevice() = default;

    /**
     * @brief Process incoming Modbus request and generate response
     * @param request Raw Modbus RTU request frame
     * @return Response frame (empty if timeout simulated)
     */
    QByteArray processRequest(const QByteArray& request);

    /**
     * @brief Set error injection configuration
     */
    void setErrorInjection(const ErrorInjectionConfig& config);

    /**
     * @brief Get slave ID
     */
    uint8_t slaveId() const { return m_slaveId; }

    /**
     * @brief Set holding register value
     */
    void setHoldingRegister(uint16_t address, uint16_t value);

    /**
     * @brief Set input register value
     */
    void setInputRegister(uint16_t address, uint16_t value);

    /**
     * @brief Set coil value
     */
    void setCoil(uint16_t address, bool value);

    /**
     * @brief Get holding register value
     */
    uint16_t getHoldingRegister(uint16_t address) const;

    /**
     * @brief Get input register value
     */
    uint16_t getInputRegister(uint16_t address) const;

    /**
     * @brief Get coil value
     */
    bool getCoil(uint16_t address) const;

protected:
    /**
     * @brief Override to handle device-specific register updates
     * Called when a write operation occurs
     */
    virtual void onRegisterWrite(uint16_t address, uint16_t value);

    /**
     * @brief Override to handle device-specific coil updates
     */
    virtual void onCoilWrite(uint16_t address, bool value);

    /**
     * @brief Override to update dynamic register values before read
     */
    virtual void updateDynamicRegisters();

private:
    uint8_t m_slaveId;
    ErrorInjectionConfig m_errorConfig;
    QMap<uint16_t, uint16_t> m_holdingRegisters;
    QMap<uint16_t, uint16_t> m_inputRegisters;
    QMap<uint16_t, bool> m_coils;

    QByteArray handleReadCoils(const QByteArray& request);
    QByteArray handleReadHoldingRegisters(const QByteArray& request);
    QByteArray handleReadInputRegisters(const QByteArray& request);
    QByteArray handleWriteSingleCoil(const QByteArray& request);
    QByteArray handleWriteSingleRegister(const QByteArray& request);
    QByteArray handleWriteMultipleRegisters(const QByteArray& request);
    QByteArray buildExceptionResponse(uint8_t functionCode, uint8_t exceptionCode);
    bool shouldInjectError(double rate);
    QByteArray corruptCRC(const QByteArray& frame);
};

} // namespace Simulation
} // namespace Infrastructure

#endif // MOCKMODBUSDEVICE_H
