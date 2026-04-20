#ifndef IBUSCONTROLLER_H
#define IBUSCONTROLLER_H

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QSerialPort>
#include <functional>

namespace Infrastructure {
namespace Bus {

/**
* @brief Abstract interface for serial bus communication
*
* Provides async request/response pattern for Modbus RTU and similar protocols.
*/
class IBusController : public QObject {
Q_OBJECT

public:
explicit IBusController(QObject* parent = nullptr) : QObject(parent) {}
virtual ~IBusController() = default;

/**
* @brief Open the bus connection
* @param portName Serial port name (e.g., "COM3", "/dev/ttyUSB0")
* @param baudRate Baud rate
* @param timeoutMs Response timeout in milliseconds
* @return true if opened successfully
*/
virtual bool open(const QString& portName,
int baudRate,
int timeoutMs,
const QString& parity = "None",
int stopBits = 1) = 0;

/**
* @brief Close the bus connection
*/
virtual void close() = 0;

/**
* @brief Check if bus is open and ready
*/
virtual bool isOpen() const = 0;

/**
* @brief Send request and wait for response synchronously
* @param request Request frame
* @param response Output buffer for response
* @return true if transaction succeeded
*/
virtual bool sendRequest(const QByteArray& request, QByteArray& response) = 0;

/**
* @brief Get last error message
*/
virtual QString lastError() const = 0;

/**
* @brief Get underlying serial port for advanced operations
* This allows device implementations to access serial port directly when needed
* without breaking abstraction layer
* @return QSerialPort pointer or nullptr if not available
*/
virtual QSerialPort* underlyingSerialPort() const { return nullptr; }

signals:
void errorOccurred(const QString& error);
};

} // namespace Bus
} // namespace Infrastructure

#endif // IBUSCONTROLLER_H
