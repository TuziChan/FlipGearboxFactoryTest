#include "MockSerialBusController.h"
#include <QThread>

namespace Infrastructure {
namespace Simulation {

MockSerialBusController::MockSerialBusController(QObject* parent)
    : IBusController(parent)
{
}

MockSerialBusController::~MockSerialBusController() {
    // Clean up all registered devices
    for (auto it = m_ports.begin(); it != m_ports.end(); ++it) {
        if (it.value().device) {
            delete it.value().device;
        }
    }
}

void MockSerialBusController::registerDevice(const QString& portName, MockModbusDevice* device) {
    QMutexLocker locker(&m_mutex);
    
    // Clean up existing device if any
    if (m_ports.contains(portName) && m_ports[portName].device) {
        delete m_ports[portName].device;
    }

    PortState state;
    state.device = device;
    state.isOpen = false;
    m_ports[portName] = state;

    // Set device parent for proper cleanup
    if (device) {
        device->setParent(this);
    }
}

void MockSerialBusController::unregisterDevice(const QString& portName) {
    QMutexLocker locker(&m_mutex);
    
    if (m_ports.contains(portName)) {
        if (m_ports[portName].device) {
            delete m_ports[portName].device;
        }
        m_ports.remove(portName);
    }
}

MockModbusDevice* MockSerialBusController::getDevice(const QString& portName) const {
    QMutexLocker locker(&m_mutex);
    
    if (m_ports.contains(portName)) {
        return m_ports[portName].device;
    }
    return nullptr;
}

void MockSerialBusController::setGlobalErrorInjection(const MockModbusDevice::ErrorInjectionConfig& config) {
    QMutexLocker locker(&m_mutex);
    
    for (auto it = m_ports.begin(); it != m_ports.end(); ++it) {
        if (it.value().device) {
            it.value().device->setErrorInjection(config);
        }
    }
}

void MockSerialBusController::setPortErrorInjection(const QString& portName,
                                                     const MockModbusDevice::ErrorInjectionConfig& config) {
    QMutexLocker locker(&m_mutex);
    
    if (m_ports.contains(portName) && m_ports[portName].device) {
        m_ports[portName].device->setErrorInjection(config);
    }
}

bool MockSerialBusController::open(const QString& portName,
                                    int baudRate,
                                    int timeoutMs,
                                    const QString& parity,
                                    int stopBits) {
    QMutexLocker locker(&m_mutex);
    
    if (!m_ports.contains(portName)) {
        setError(QString("Port %1 not registered").arg(portName));
        return false;
    }

    if (!m_ports[portName].device) {
        setError(QString("No device registered on port %1").arg(portName));
        return false;
    }

    m_ports[portName].isOpen = true;
    m_ports[portName].baudRate = baudRate;
    m_ports[portName].timeoutMs = timeoutMs;
    m_ports[portName].parity = parity;
    m_ports[portName].stopBits = stopBits;
    m_currentPort = portName;

    return true;
}

void MockSerialBusController::close() {
    QMutexLocker locker(&m_mutex);
    
    if (!m_currentPort.isEmpty() && m_ports.contains(m_currentPort)) {
        m_ports[m_currentPort].isOpen = false;
    }
    m_currentPort.clear();
}

bool MockSerialBusController::isOpen() const {
    QMutexLocker locker(&m_mutex);
    
    if (m_currentPort.isEmpty()) {
        return false;
    }

    if (!m_ports.contains(m_currentPort)) {
        return false;
    }

    return m_ports[m_currentPort].isOpen;
}

bool MockSerialBusController::sendRequest(const QByteArray& request, QByteArray& response) {
    QMutexLocker locker(&m_mutex);
    
    if (m_currentPort.isEmpty()) {
        setError("No port opened");
        return false;
    }

    if (!m_ports.contains(m_currentPort)) {
        setError(QString("Port %1 not found").arg(m_currentPort));
        return false;
    }

    const PortState& port = m_ports[m_currentPort];

    if (!port.isOpen) {
        setError(QString("Port %1 is not open").arg(m_currentPort));
        return false;
    }

    if (!port.device) {
        setError(QString("No device on port %1").arg(m_currentPort));
        return false;
    }

    // Simulate transmission delay based on baud rate
    // Approximate: 10 bits per byte (8 data + 1 start + 1 stop)
    int bytesToSend = request.size();
    int transmissionTimeMs = (bytesToSend * 10 * 1000) / port.baudRate;
    if (transmissionTimeMs > 0) {
        QThread::msleep(qMin(transmissionTimeMs, 100)); // Cap at 100ms
    }

    // Process request through mock device
    response = port.device->processRequest(request);

    // Check for timeout (empty response)
    if (response.isEmpty()) {
        setError("Device timeout - no response");
        return false;
    }

    // Simulate response reception delay
    int responseTimeMs = (response.size() * 10 * 1000) / port.baudRate;
    if (responseTimeMs > 0) {
        QThread::msleep(qMin(responseTimeMs, 100)); // Cap at 100ms
    }

    return true;
}

QString MockSerialBusController::lastError() const {
    QMutexLocker locker(&m_mutex);
    return m_lastError;
}

void MockSerialBusController::setError(const QString& error) {
    m_lastError = error;
}

} // namespace Simulation
} // namespace Infrastructure
