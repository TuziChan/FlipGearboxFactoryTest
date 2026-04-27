#include "Dyn200ProactiveListener.h"
#include <QDebug>

namespace Infrastructure {
namespace Devices {

Dyn200ProactiveListener::Dyn200ProactiveListener(ProtocolMode mode, QObject* parent)
    : QObject(parent)
    , m_serialPort(nullptr)
    , m_mode(mode)
    , m_buffer()
    , m_running(false)
    , m_latestTorque(0.0)
    , m_latestSpeed(0.0)
    , m_speedValid(mode != Ascii)
{
}

Dyn200ProactiveListener::~Dyn200ProactiveListener() {
    stop();
}

void Dyn200ProactiveListener::start(QSerialPort* serialPort) {
    // Use mutex to ensure atomic check-and-set of m_running
    QMutexLocker locker(&m_mutex);
    
    if (m_running.load()) {
        qWarning() << "Dyn200ProactiveListener already running, ignoring duplicate start()";
        return;
    }
    if (!serialPort || !serialPort->isOpen()) {
        locker.unlock();
        emit errorOccurred("Serial port is not open");
        return;
    }

    m_serialPort = serialPort;
    m_buffer.clear();
    m_running.store(true);
    
    // Connect outside the lock - Qt signal connection is thread-safe
    locker.unlock();
    
    connect(m_serialPort, &QSerialPort::readyRead,
            this, &Dyn200ProactiveListener::onReadyRead,
            Qt::UniqueConnection);  // Prevent duplicate connections
    
    qDebug() << "Dyn200ProactiveListener started in mode" << m_mode;
}

void Dyn200ProactiveListener::stop() {
    // First atomically set m_running to false to prevent new onReadyRead() from proceeding
    if (!m_running.load()) {
        return;
    }
    m_running.store(false);

    // Disconnect signal handler - this ensures no more onReadyRead() will be invoked
    // after this point (already queued callbacks may still run but will exit early)
    QMutexLocker locker(&m_mutex);
    if (m_serialPort) {
        disconnect(m_serialPort, &QSerialPort::readyRead,
                   this, &Dyn200ProactiveListener::onReadyRead);
        m_serialPort = nullptr;
    }
    m_buffer.clear();
}

void Dyn200ProactiveListener::onReadyRead() {
    // Quick check without lock - m_running is atomic
    if (!m_running.load()) {
        return;
    }

    QMutexLocker locker(&m_mutex);
    
    // Double-check after acquiring lock
    if (!m_serialPort || !m_running.load()) {
        return;
    }

    // Read and append data under lock protection
    m_buffer.append(m_serialPort->readAll());

    switch (m_mode) {
        case Hex6Byte:
            parseHex6ByteFrame();
            break;
        case Hex8Byte:
            parseHex8ByteFrame();
            break;
        case Ascii:
            parseAsciiFrame();
            break;
    }
}

void Dyn200ProactiveListener::parseHex6ByteFrame() {
    // Frame format: D1D2 (torque uint16) + D3D4 (speed int16, D3 MSB indicates torque sign) + D5D6 (CRC16)
    while (m_buffer.size() >= 6) {
        QByteArray frame = m_buffer.left(6);
        
        // Verify CRC
        QByteArray dataWithoutCRC = frame.left(4);
        uint16_t receivedCRC = static_cast<uint8_t>(frame[4]) | (static_cast<uint8_t>(frame[5]) << 8);
        uint16_t calculatedCRC = calculateCRC16(dataWithoutCRC);
        
        if (receivedCRC != calculatedCRC) {
            emit errorOccurred(QString("HEX 6-byte CRC error: expected 0x%1, got 0x%2")
                              .arg(calculatedCRC, 4, 16, QChar('0'))
                              .arg(receivedCRC, 4, 16, QChar('0')));
            m_buffer.remove(0, 1); // Remove one byte and try again
            continue;
        }
        
        // Parse torque (D1D2)
        uint16_t torqueRaw = static_cast<uint8_t>(frame[0]) | (static_cast<uint8_t>(frame[1]) << 8);
        
        // Parse speed and torque sign (D3D4)
        uint8_t d3 = static_cast<uint8_t>(frame[2]);
        uint8_t d4 = static_cast<uint8_t>(frame[3]);
        
        // D3 MSB indicates torque sign
        bool torqueNegative = (d3 & 0x80) != 0;

        // Speed is int16 (D3D4), mask out D3 MSB (torque sign bit) to avoid sign contamination
        int16_t speedRaw = static_cast<int16_t>((d3 & 0x7F) | (d4 << 8));
        
        // Apply scaling
        double torque = torqueRaw * 0.01; // Scale to N·m
        if (torqueNegative) {
            torque = -torque;
        }
        double speed = speedRaw; // Already in RPM

        m_latestTorque.store(torque);
        m_latestSpeed.store(speed);
        m_speedValid.store(true);

        emit dataReceived(torque, speed, true);
        
        // Remove processed frame
        m_buffer.remove(0, 6);
    }
}

void Dyn200ProactiveListener::parseHex8ByteFrame() {
    // Frame format: D1D2D3 (torque 3-byte signed) + D4D5D6 (speed 3-byte unsigned) + D7D8 (CRC16)
    while (m_buffer.size() >= 8) {
        QByteArray frame = m_buffer.left(8);
        
        // Verify CRC
        QByteArray dataWithoutCRC = frame.left(6);
        uint16_t receivedCRC = static_cast<uint8_t>(frame[6]) | (static_cast<uint8_t>(frame[7]) << 8);
        uint16_t calculatedCRC = calculateCRC16(dataWithoutCRC);
        
        if (receivedCRC != calculatedCRC) {
            emit errorOccurred(QString("HEX 8-byte CRC error: expected 0x%1, got 0x%2")
                              .arg(calculatedCRC, 4, 16, QChar('0'))
                              .arg(receivedCRC, 4, 16, QChar('0')));
            m_buffer.remove(0, 1); // Remove one byte and try again
            continue;
        }
        
        // Parse torque (D1D2D3) - 3-byte signed integer
        uint8_t d1 = static_cast<uint8_t>(frame[0]);
        uint8_t d2 = static_cast<uint8_t>(frame[1]);
        uint8_t d3 = static_cast<uint8_t>(frame[2]);
        
        int32_t torqueRaw = d1 | (d2 << 8) | (d3 << 16);
        // Sign extend if negative (MSB of D3 is 1)
        if (d3 & 0x80) {
            torqueRaw |= 0xFF000000; // Sign extend to 32-bit
        }
        
        // Parse speed (D4D5D6) - 3-byte unsigned integer
        uint8_t d4 = static_cast<uint8_t>(frame[3]);
        uint8_t d5 = static_cast<uint8_t>(frame[4]);
        uint8_t d6 = static_cast<uint8_t>(frame[5]);
        
        uint32_t speedRaw = d4 | (d5 << 8) | (d6 << 16);
        
        // Apply scaling
        double torque = torqueRaw * 0.01; // Scale to N·m
        double speed = speedRaw; // Already in RPM

        m_latestTorque.store(torque);
        m_latestSpeed.store(speed);
        m_speedValid.store(true);

        emit dataReceived(torque, speed, true);
        
        // Remove processed frame
        m_buffer.remove(0, 8);
    }
}

void Dyn200ProactiveListener::parseAsciiFrame() {
    // Frame format: [+-]X.XXXX\r (ASCII string terminated by 0x0D)
    int crIndex = m_buffer.indexOf('\r');
    while (crIndex >= 0) {
        QByteArray frame = m_buffer.left(crIndex);
        
        // Parse ASCII torque value
        QString torqueStr = QString::fromLatin1(frame).trimmed();
        bool ok = false;
        double torque = torqueStr.toDouble(&ok);
        
        if (ok) {
            m_latestTorque.store(torque);
            m_speedValid.store(false); // ASCII mode doesn't provide speed

            emit dataReceived(torque, 0.0, false);
        } else {
            emit errorOccurred(QString("Failed to parse ASCII torque: '%1'").arg(torqueStr));
        }
        
        // Remove processed frame including \r
        m_buffer.remove(0, crIndex + 1);
        
        // Look for next frame
        crIndex = m_buffer.indexOf('\r');
    }
    
    // Prevent buffer overflow - keep only last 32 bytes if no \r found
    if (m_buffer.size() > 32) {
        m_buffer.remove(0, m_buffer.size() - 32);
    }
}

uint16_t Dyn200ProactiveListener::calculateCRC16(const QByteArray& data) const {
    uint16_t crc = 0xFFFF;
    
    for (int i = 0; i < data.size(); ++i) {
        crc ^= static_cast<uint8_t>(data[i]);
        
        for (int j = 0; j < 8; ++j) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    
    return crc;
}

} // namespace Devices
} // namespace Infrastructure
