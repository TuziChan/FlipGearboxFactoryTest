#ifndef SIMULATEDBUSCONTROLLER_H
#define SIMULATEDBUSCONTROLLER_H

#include "src/infrastructure/bus/IBusController.h"

namespace Infrastructure {
namespace Simulation {

/**
 * @brief Simulated bus controller that always succeeds
 * 
 * This mock bus controller is used in simulation mode to bypass actual serial communication.
 */
class SimulatedBusController : public Bus::IBusController {
    Q_OBJECT

public:
    explicit SimulatedBusController(QObject* parent = nullptr)
        : IBusController(parent)
        , m_isOpen(false)
    {
    }

    bool open(const QString& portName,
              int baudRate,
              int timeoutMs,
              const QString& parity = "None",
              int stopBits = 1) override {
        m_portName = portName;
        m_baudRate = baudRate;
        m_timeoutMs = timeoutMs;
        m_parity = parity;
        m_stopBits = stopBits;
        m_isOpen = true;
        return true;
    }

    void close() override {
        m_isOpen = false;
    }

    bool isOpen() const override {
        return m_isOpen;
    }

    bool sendRequest(const QByteArray& request, QByteArray& response) override {
        // Always succeed with empty response
        response.clear();
        return true;
    }

    QString lastError() const override {
        return QString();
    }

private:
    bool m_isOpen;
    QString m_portName;
    int m_baudRate;
    int m_timeoutMs;
    QString m_parity;
    int m_stopBits;
};

} // namespace Simulation
} // namespace Infrastructure

#endif // SIMULATEDBUSCONTROLLER_H
