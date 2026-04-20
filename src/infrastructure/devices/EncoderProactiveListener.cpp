#include "EncoderProactiveListener.h"
#include <QDebug>

namespace Infrastructure {
namespace Devices {

EncoderProactiveListener::EncoderProactiveListener(QSerialPort* serialPort, QObject* parent)
    : QObject(parent)
    , m_serialPort(serialPort)
{
}

EncoderProactiveListener::~EncoderProactiveListener() {
    stop();
}

void EncoderProactiveListener::start() {
    if (!m_serialPort || !m_serialPort->isOpen()) {
        emit errorOccurred("Serial port is not open");
        return;
    }

    m_buffer.clear();
    m_valid.store(false);

    connect(m_serialPort, &QSerialPort::readyRead, this, &EncoderProactiveListener::onReadyRead);

    qDebug() << "EncoderProactiveListener started";
}

void EncoderProactiveListener::stop() {
    if (m_serialPort) {
        disconnect(m_serialPort, &QSerialPort::readyRead, this, &EncoderProactiveListener::onReadyRead);
    }
    m_buffer.clear();
    m_valid.store(false);
}

void EncoderProactiveListener::onReadyRead() {
    if (!m_serialPort) {
        return;
    }

    m_buffer.append(m_serialPort->readAll());

    // Encoder auto-report frame: 4 bytes (angle value, big-endian uint16)
    // Process complete frames
    while (m_buffer.size() >= 4) {
        // Parse angle from first 2 bytes (assuming single-turn mode)
        uint8_t highByte = static_cast<uint8_t>(m_buffer[0]);
        uint8_t lowByte = static_cast<uint8_t>(m_buffer[1]);
        uint16_t rawAngle = (static_cast<uint16_t>(highByte) << 8) | lowByte;

        // Convert to degrees (assuming 360 degrees = full scale)
        double angleDeg = (static_cast<double>(rawAngle) / 65535.0) * 360.0;

        m_latestAngle.store(angleDeg);
        m_valid.store(true);

        emit dataReceived(angleDeg);

        // Remove processed bytes
        m_buffer.remove(0, 2);

        // In auto-report mode, frames are continuous, so we keep processing
        // Break after one complete frame to avoid starvation
        break;
    }

    // Prevent buffer overflow
    if (m_buffer.size() > 16) {
        m_buffer.remove(0, m_buffer.size() - 8);
    }
}

double EncoderProactiveListener::latestAngle() const {
    return m_latestAngle.load();
}

bool EncoderProactiveListener::isValid() const {
    return m_valid.load();
}

} // namespace Devices
} // namespace Infrastructure
