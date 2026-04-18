#ifndef MODBUSRTUBUSCONTROLLER_H
#define MODBUSRTUBUSCONTROLLER_H

#include "IBusController.h"
#include <QSerialPort>
#include <QTimer>

namespace Infrastructure {
namespace Bus {

/**
 * @brief Modbus RTU bus controller using Qt Serial Port
 *
 * Implements synchronous request/response pattern with timeout handling.
 */
class ModbusRtuBusController : public IBusController {
    Q_OBJECT

public:
    explicit ModbusRtuBusController(QObject* parent = nullptr);
    ~ModbusRtuBusController() override;

    bool open(const QString& portName,
              int baudRate,
              int timeoutMs,
              const QString& parity = "None",
              int stopBits = 1) override;
    void close() override;
    bool isOpen() const override;
    bool sendRequest(const QByteArray& request, QByteArray& response) override;
    QString lastError() const override;

    /**
     * @brief Reconfigure serial port parameters at runtime
     * @param baudRate New baud rate
     * @param parity New parity setting ("None", "Even", "Odd")
     * @param stopBits New stop bits (1 or 2)
     * @return true if reconfiguration succeeded
     */
    bool reconfigure(int baudRate, const QString& parity, int stopBits);

    /**
     * @brief Set inter-frame delay (T3.5 character times)
     * Default is calculated based on baud rate
     */
    void setInterFrameDelayMs(int delayMs);

private:
    QSerialPort* m_serialPort;
    int m_timeoutMs;
    int m_interFrameDelayMs;
    QString m_lastError;
    
    // Cached parameters for reconfiguration when port is not open
    int m_cachedBaudRate;
    QString m_cachedParity;
    int m_cachedStopBits;

    /**
     * @brief Calculate inter-frame delay based on baud rate
     * Modbus RTU requires 3.5 character times between frames
     */
    int calculateInterFrameDelay(int baudRate) const;

    /**
     * @brief Wait for response with timeout
     */
    bool waitForResponse(QByteArray& response, int expectedMinBytes);
    QSerialPort::Parity parseParity(const QString& parity) const;
    QSerialPort::StopBits parseStopBits(int stopBits) const;
};

} // namespace Bus
} // namespace Infrastructure

#endif // MODBUSRTUBUSCONTROLLER_H
