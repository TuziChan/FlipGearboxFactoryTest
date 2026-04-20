#ifndef MOCKSERIALBUSCONTROLLER_H
#define MOCKSERIALBUSCONTROLLER_H

#include "../bus/IBusController.h"
#include "MockModbusDevice.h"
#include <QMap>
#include <QMutex>

namespace Infrastructure {
namespace Simulation {

/**
 * @brief Multi-port mock serial bus controller
 * 
 * Simulates multiple serial ports, each connected to a mock Modbus device.
 * Supports concurrent communication testing and error injection.
 */
class MockSerialBusController : public Bus::IBusController {
    Q_OBJECT

public:
    explicit MockSerialBusController(QObject* parent = nullptr);
    ~MockSerialBusController() override;

    /**
     * @brief Register a mock device on a virtual port
     * @param portName Virtual port name (e.g., "COM1", "COM2")
     * @param device Mock device instance (ownership transferred)
     */
    void registerDevice(const QString& portName, MockModbusDevice* device);

    /**
     * @brief Unregister device from a port
     */
    void unregisterDevice(const QString& portName);

    /**
     * @brief Get device registered on a port
     */
    MockModbusDevice* getDevice(const QString& portName) const;

    /**
     * @brief Set global error injection for all devices
     */
    void setGlobalErrorInjection(const MockModbusDevice::ErrorInjectionConfig& config);

    /**
     * @brief Set error injection for specific port
     */
    void setPortErrorInjection(const QString& portName, 
                               const MockModbusDevice::ErrorInjectionConfig& config);

    // IBusController interface
    bool open(const QString& portName,
              int baudRate,
              int timeoutMs,
              const QString& parity = "None",
              int stopBits = 1) override;

    void close() override;
    bool isOpen() const override;
    bool sendRequest(const QByteArray& request, QByteArray& response) override;
    QString lastError() const override;

private:
    struct PortState {
        MockModbusDevice* device = nullptr;
        bool isOpen = false;
        int baudRate = 9600;
        int timeoutMs = 1000;
        QString parity = "None";
        int stopBits = 1;
    };

    QMap<QString, PortState> m_ports;
    QString m_currentPort;
    QString m_lastError;
    mutable QMutex m_mutex;

    void setError(const QString& error);
};

} // namespace Simulation
} // namespace Infrastructure

#endif // MOCKSERIALBUSCONTROLLER_H
