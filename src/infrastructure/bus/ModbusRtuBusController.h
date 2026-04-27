#ifndef MODBUSRTUBUSCONTROLLER_H
#define MODBUSRTUBUSCONTROLLER_H

#include "IBusController.h"
#include <QSerialPort>
#include <QMutex>

namespace Infrastructure {
namespace Bus {

/**
 * @brief Modbus RTU bus controller using Qt Serial Port
 *
 * Thread-safe synchronous implementation. QSerialPort is created in the main thread
 * and accessed through a mutex to prevent concurrent access from Poller threads.
 *
 * Note: According to Qt best practices, QSerialPort is already asynchronous and
 * typically doesn't require a separate thread. This implementation uses a mutex
 * to serialize access from multiple Poller threads.
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

    bool reconfigure(int baudRate, const QString& parity, int stopBits);
    void setInterFrameDelayMs(int delayMs);
    QSerialPort* underlyingSerialPort() const override;

private:
    QSerialPort* m_serialPort;
    int m_timeoutMs;
    int m_interFrameDelayMs;
    QString m_lastError;
    mutable QMutex m_mutex; // Protects all serial port operations

    int calculateInterFrameDelay(int baudRate) const;
    bool waitForResponse(const QByteArray& request, QByteArray& response);
    QSerialPort::Parity parseParity(const QString& parity) const;
    QSerialPort::StopBits parseStopBits(int stopBits) const;
};

} // namespace Bus
} // namespace Infrastructure

#endif // MODBUSRTUBUSCONTROLLER_H
