#include "EncoderProactiveListener.h"
#include <QDebug>

namespace Infrastructure {
namespace Devices {

EncoderProactiveListener::EncoderProactiveListener(QSerialPort* serialPort,
                                                    ProtocolMode mode,
                                                    uint16_t resolution,
                                                    QObject* parent)
    : QObject(parent)
    , m_serialPort(serialPort)
    , m_mode(mode)
    , m_resolution(resolution)
{
}

EncoderProactiveListener::~EncoderProactiveListener() {
    stop();
}

void EncoderProactiveListener::start() {
    if (m_running.load()) {
        qWarning() << "EncoderProactiveListener already running, ignoring duplicate start()";
        return;
    }
    if (!m_serialPort || !m_serialPort->isOpen()) {
        emit errorOccurred("Serial port is not open");
        return;
    }

    m_buffer.clear();
    m_valid.store(false);
    m_running.store(true);

    connect(m_serialPort, &QSerialPort::readyRead,
            this, &EncoderProactiveListener::onReadyRead,
            Qt::UniqueConnection);  // Prevent duplicate connections

    qDebug() << "EncoderProactiveListener started, mode:" << static_cast<int>(m_mode)
             << "resolution:" << m_resolution;
}

void EncoderProactiveListener::stop() {
    if (!m_running.load()) {
        return;
    }
    m_running.store(false);

    if (m_serialPort) {
        disconnect(m_serialPort, &QSerialPort::readyRead,
                   this, &EncoderProactiveListener::onReadyRead);
    }
    m_buffer.clear();
    m_valid.store(false);
}

void EncoderProactiveListener::onReadyRead() {
    if (!m_serialPort || !m_running.load()) {
        return;
    }

    m_buffer.append(m_serialPort->readAll());

    switch (m_mode) {
        case ProtocolMode::SingleTurn:
            parseSingleTurnFrame();
            break;
        case ProtocolMode::MultiTurn:
            parseMultiTurnFrame();
            break;
        case ProtocolMode::Velocity:
            parseVelocityFrame();
            break;
    }

    // Prevent buffer overflow
    const int MAX_BUFFER_SIZE = 64;
    if (m_buffer.size() > MAX_BUFFER_SIZE) {
        qWarning() << "EncoderProactiveListener buffer overflow, dropping"
                   << m_buffer.size() - MAX_BUFFER_SIZE / 2 << "bytes";
        m_buffer.remove(0, m_buffer.size() - MAX_BUFFER_SIZE / 2);
    }
}

// Mode 2: Single-turn value (2 bytes, uint16 BE)
void EncoderProactiveListener::parseSingleTurnFrame() {
    while (m_buffer.size() >= 2) {
        uint8_t highByte = static_cast<uint8_t>(m_buffer[0]);
        uint8_t lowByte  = static_cast<uint8_t>(m_buffer[1]);
        uint16_t rawAngle = (static_cast<uint16_t>(highByte) << 8) | lowByte;

        double angleDeg = (static_cast<double>(rawAngle) / static_cast<double>(m_resolution)) * 360.0;

        m_latestAngle.store(angleDeg);
        m_valid.store(true);
        emit dataReceived(angleDeg);

        m_buffer.remove(0, 2);
    }
}

// Mode 3: Virtual multi-turn value
// Note: Frame length assumption — see hardware verification section below
void EncoderProactiveListener::parseMultiTurnFrame() {
    // Assumption A: 3 bytes (24-bit) based on register max 0xFFFFFF
    // Assumption B: 4 bytes (32-bit, padded) if encoder sends full uint32
    // Default implementation uses 4 bytes for safety; adjust after HW test.
    const int FRAME_SIZE = 4;  // TODO: verify with hardware

    while (m_buffer.size() >= FRAME_SIZE) {
        uint32_t rawValue = 0;
        for (int i = 0; i < FRAME_SIZE; ++i) {
            rawValue = (rawValue << 8) | static_cast<uint8_t>(m_buffer[i]);
        }

        // If 4-byte frame and encoder only uses 24-bit, mask upper byte
        if (FRAME_SIZE == 4 && rawValue > 0xFFFFFF) {
            rawValue &= 0xFFFFFF;
        }

        // Convert accumulated count to total degrees
        double totalAngleDeg = (static_cast<double>(rawValue) / static_cast<double>(m_resolution)) * 360.0;

        m_latestMultiTurn.store(totalAngleDeg);
        m_valid.store(true);
        emit dataReceived(totalAngleDeg);

        m_buffer.remove(0, FRAME_SIZE);
    }
}

// Mode 4: Angular velocity (2 bytes, int16 BE)
void EncoderProactiveListener::parseVelocityFrame() {
    while (m_buffer.size() >= 2) {
        uint8_t highByte = static_cast<uint8_t>(m_buffer[0]);
        uint8_t lowByte  = static_cast<uint8_t>(m_buffer[1]);
        int16_t rawVelocity = static_cast<int16_t>((static_cast<uint16_t>(highByte) << 8) | lowByte);

        // Scaling: raw velocity -> RPM (device-specific; verify with manual)
        // Default: direct value as RPM. Adjust if manual specifies different scaling.
        double velocityRpm = static_cast<double>(rawVelocity);

        m_latestVelocity.store(velocityRpm);
        m_valid.store(true);
        emit dataReceived(velocityRpm);

        m_buffer.remove(0, 2);
    }
}

double EncoderProactiveListener::latestAngle() const {
    return m_latestAngle.load();
}

double EncoderProactiveListener::latestMultiTurn() const {
    return m_latestMultiTurn.load();
}

double EncoderProactiveListener::latestVelocity() const {
    return m_latestVelocity.load();
}

bool EncoderProactiveListener::isValid() const {
    return m_valid.load();
}

EncoderProactiveListener::ProtocolMode EncoderProactiveListener::mode() const {
    return m_mode;
}

} // namespace Devices
} // namespace Infrastructure
