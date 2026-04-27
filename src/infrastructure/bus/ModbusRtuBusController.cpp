#include "ModbusRtuBusController.h"
#include "ModbusFrame.h"
#include <QThread>
#include <QElapsedTimer>
#include <QMutexLocker>
#include <QDebug>

namespace Infrastructure {
namespace Bus {

ModbusRtuBusController::ModbusRtuBusController(QObject* parent)
    : IBusController(parent)
    , m_serialPort(new QSerialPort(this))
    , m_timeoutMs(500)
    , m_interFrameDelayMs(2)
    , m_lastError()
{
    qDebug() << "[ModbusRtuBusController] Created in thread" << QThread::currentThreadId();
}

ModbusRtuBusController::~ModbusRtuBusController() {
    close();
}

bool ModbusRtuBusController::open(const QString& portName, int baudRate, int timeoutMs, const QString& parity, int stopBits) {
    QMutexLocker locker(&m_mutex);

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

    if (!m_serialPort->open(QIODevice::ReadWrite)) {
        m_lastError = QString("Failed to open port %1: %2")
                          .arg(portName)
                          .arg(m_serialPort->errorString());
        emit errorOccurred(m_lastError);
        return false;
    }

    m_serialPort->clear();

    qDebug() << "[ModbusRtuBusController] Port opened:" << portName
             << "at" << baudRate << "baud"
             << "parity" << parity
             << "stopBits" << stopBits
             << "with" << m_interFrameDelayMs << "ms inter-frame delay";

    return true;
}

void ModbusRtuBusController::close() {
    QMutexLocker locker(&m_mutex);
    if (m_serialPort->isOpen()) {
        m_serialPort->close();
    }
}

bool ModbusRtuBusController::isOpen() const {
    QMutexLocker locker(&m_mutex);
    return m_serialPort->isOpen();
}

bool ModbusRtuBusController::sendRequest(const QByteArray& request, QByteArray& response) {
    QMutexLocker locker(&m_mutex);

    if (!m_serialPort->isOpen()) {
        m_lastError = "Serial port is not open";
        return false;
    }

    m_serialPort->clear();
    QThread::msleep(m_interFrameDelayMs);

    // Log raw request frame
    QString requestHex;
    for (int i = 0; i < request.size(); ++i) {
        requestHex += QString("%1 ").arg(static_cast<uint8_t>(request[i]), 2, 16, QChar('0')).toUpper();
    }
    qDebug() << "[Modbus TX]" << requestHex.trimmed();

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

    if (!waitForResponse(request, response)) {
        return false;
    }

    // Log raw response frame
    QString responseHex;
    for (int i = 0; i < response.size(); ++i) {
        responseHex += QString("%1 ").arg(static_cast<uint8_t>(response[i]), 2, 16, QChar('0')).toUpper();
    }
    qDebug() << "[Modbus RX]" << responseHex.trimmed();

    return true;
}

QString ModbusRtuBusController::lastError() const {
    QMutexLocker locker(&m_mutex);
    return m_lastError;
}

bool ModbusRtuBusController::reconfigure(int baudRate, const QString& parity, int stopBits) {
    QMutexLocker locker(&m_mutex);

    if (!m_serialPort->isOpen()) {
        m_lastError = "Port is not open";
        return false;
    }

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

    m_interFrameDelayMs = calculateInterFrameDelay(baudRate);

    qDebug() << "[ModbusRtuBusController] Reconfigured:"
             << "baudRate" << baudRate
             << "parity" << parity
             << "stopBits" << stopBits
             << "interFrameDelay" << m_interFrameDelayMs << "ms";

    return true;
}

void ModbusRtuBusController::setInterFrameDelayMs(int delayMs) {
    QMutexLocker locker(&m_mutex);
    m_interFrameDelayMs = delayMs;
}

int ModbusRtuBusController::calculateInterFrameDelay(int baudRate) const {
    if (baudRate >= 19200) {
        return 2;
    }
    int delayMs = (3.5 * 11 * 1000) / baudRate;
    return qMax(2, delayMs);
}

QSerialPort* ModbusRtuBusController::underlyingSerialPort() const {
    return m_serialPort;
}

bool ModbusRtuBusController::waitForResponse(const QByteArray& request, QByteArray& response) {
    response.clear();

    QElapsedTimer timer;
    timer.start();

    int expectedLength = -1;

    while (timer.elapsed() < m_timeoutMs) {
        if (!m_serialPort->waitForReadyRead(50)) {
            continue;
        }

        response.append(m_serialPort->readAll());
        while (m_serialPort->bytesAvailable() > 0) {
            response.append(m_serialPort->readAll());
        }

        expectedLength = ModbusFrame::tryGetExpectedResponseLength(request, response);
        if (expectedLength > 0 && response.size() >= expectedLength) {
            return true;
        }
    }

    if (response.isEmpty()) {
        m_lastError = QString("Timeout: no response received within %1 ms").arg(m_timeoutMs);
    } else if (expectedLength > 0) {
        m_lastError = QString("Timeout: incomplete response (received %1 bytes, expected %2)")
                          .arg(response.size())
                          .arg(expectedLength);
    } else {
        m_lastError = QString("Timeout: incomplete response (received %1 bytes, frame length undetermined)")
                          .arg(response.size());
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
