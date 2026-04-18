#include "ModbusRtuBusController.h"
#include <QThread>
#include <QElapsedTimer>
#include <QDebug>

namespace Infrastructure {
namespace Bus {

ModbusRtuBusController::ModbusRtuBusController(QObject* parent)
    : IBusController(parent)
    , m_serialPort(new QSerialPort(this))
    , m_timeoutMs(500)
    , m_interFrameDelayMs(2)
    , m_lastError()
    , m_cachedBaudRate(9600)
    , m_cachedParity("None")
    , m_cachedStopBits(1)
{
}

ModbusRtuBusController::~ModbusRtuBusController() {
    close();
}

bool ModbusRtuBusController::open(const QString& portName, int baudRate, int timeoutMs, const QString& parity, int stopBits) {
    if (m_serialPort->isOpen()) {
        close();
    }

    m_serialPort->setPortName(portName);
    m_serialPort->setBaudRate(baudRate);
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setParity(parseParity(parity));
    m_serialPort->setStopBits(parseStopBits(stopBits));
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

    m_timeoutMs = timeoutMs;
    m_interFrameDelayMs = calculateInterFrameDelay(baudRate);
    
    // Cache parameters
    m_cachedBaudRate = baudRate;
    m_cachedParity = parity;
    m_cachedStopBits = stopBits;

    if (!m_serialPort->open(QIODevice::ReadWrite)) {
        m_lastError = QString("Failed to open port %1: %2")
                          .arg(portName)
                          .arg(m_serialPort->errorString());
        emit errorOccurred(m_lastError);
        return false;
    }

    m_serialPort->clear();

    qDebug() << "Modbus RTU bus opened:" << portName
             << "at" << baudRate << "baud"
             << "parity" << parity
             << "stopBits" << stopBits
             << "with" << m_interFrameDelayMs << "ms inter-frame delay";

    return true;
}

void ModbusRtuBusController::close() {
    if (m_serialPort->isOpen()) {
        m_serialPort->close();
    }
}

bool ModbusRtuBusController::isOpen() const {
    return m_serialPort->isOpen();
}

bool ModbusRtuBusController::sendRequest(const QByteArray& request, QByteArray& response) {
    if (!m_serialPort->isOpen()) {
        m_lastError = "Serial port is not open";
        return false;
    }

    m_serialPort->clear();
    QThread::msleep(m_interFrameDelayMs);

    qint64 bytesWritten = m_serialPort->write(request);
    if (bytesWritten != request.size()) {
        m_lastError = QString("Failed to write complete request: wrote %1 of %2 bytes")
                          .arg(bytesWritten)
                          .arg(request.size());
        return false;
    }

    if (!m_serialPort->waitForBytesWritten(m_timeoutMs)) {
        m_lastError = "Timeout waiting for bytes to be written";
        return false;
    }

    if (!waitForResponse(response, 5)) {
        return false;
    }

    return true;
}

QString ModbusRtuBusController::lastError() const {
    return m_lastError;
}

bool ModbusRtuBusController::reconfigure(int baudRate, const QString& parity, int stopBits) {
    // Cache parameters for future use
    m_cachedBaudRate = baudRate;
    m_cachedParity = parity;
    m_cachedStopBits = stopBits;
    
    if (m_serialPort->isOpen()) {
        // Apply immediately if port is open
        if (!m_serialPort->setBaudRate(baudRate)) {
            m_lastError = QString("Failed to set baud rate to %1").arg(baudRate);
            return false;
        }
        
        if (!m_serialPort->setParity(parseParity(parity))) {
            m_lastError = QString("Failed to set parity to %1").arg(parity);
            return false;
        }
        
        if (!m_serialPort->setStopBits(parseStopBits(stopBits))) {
            m_lastError = QString("Failed to set stop bits to %1").arg(stopBits);
            return false;
        }
        
        // Update inter-frame delay based on new baud rate
        m_interFrameDelayMs = calculateInterFrameDelay(baudRate);
        
        qDebug() << "Modbus RTU bus reconfigured:"
                 << "baudRate" << baudRate
                 << "parity" << parity
                 << "stopBits" << stopBits
                 << "interFrameDelay" << m_interFrameDelayMs << "ms";
    } else {
        // Port not open, parameters will be used on next open()
        qDebug() << "Modbus RTU bus parameters cached for next open:"
                 << "baudRate" << baudRate
                 << "parity" << parity
                 << "stopBits" << stopBits;
    }
    
    return true;
}

void ModbusRtuBusController::setInterFrameDelayMs(int delayMs) {
    m_interFrameDelayMs = delayMs;
}

int ModbusRtuBusController::calculateInterFrameDelay(int baudRate) const {
    if (baudRate >= 19200) {
        return 2;
    }

    int delayMs = (3.5 * 11 * 1000) / baudRate;
    return qMax(2, delayMs);
}

bool ModbusRtuBusController::waitForResponse(QByteArray& response, int expectedMinBytes) {
    response.clear();

    QElapsedTimer timer;
    timer.start();

    while (timer.elapsed() < m_timeoutMs) {
        if (m_serialPort->waitForReadyRead(50)) {
            response.append(m_serialPort->readAll());

            if (response.size() >= expectedMinBytes) {
                QThread::msleep(5);
                if (m_serialPort->bytesAvailable() > 0) {
                    response.append(m_serialPort->readAll());
                }
                return true;
            }
        }
    }

    if (response.isEmpty()) {
        m_lastError = QString("Timeout: no response received within %1 ms").arg(m_timeoutMs);
    } else {
        m_lastError = QString("Timeout: incomplete response (received %1 bytes, expected at least %2)")
                          .arg(response.size())
                          .arg(expectedMinBytes);
    }

    return false;
}

QSerialPort::Parity ModbusRtuBusController::parseParity(const QString& parity) const {
    const QString normalized = parity.trimmed().toLower();
    if (normalized == "even")
        return QSerialPort::EvenParity;
    if (normalized == "odd")
        return QSerialPort::OddParity;
    return QSerialPort::NoParity;
}

QSerialPort::StopBits ModbusRtuBusController::parseStopBits(int stopBits) const {
    return stopBits == 2 ? QSerialPort::TwoStop : QSerialPort::OneStop;
}

} // namespace Bus
} // namespace Infrastructure
