#ifndef ENCODERPROACTIVELISTENER_H
#define ENCODERPROACTIVELISTENER_H

#include <QObject>
#include <QSerialPort>
#include <QByteArray>
#include <atomic>

namespace Infrastructure {
namespace Devices {

/**
 * @brief Listener for encoder proactive upload modes
 *
 * Handles auto-report modes:
 * - SingleTurn (Mode 2): 2-byte uint16 big-endian angle count
 * - MultiTurn  (Mode 3): 3~4-byte uint32 big-endian accumulated count
 * - Velocity   (Mode 4): 2-byte int16 big-endian angular velocity
 */
class EncoderProactiveListener : public QObject {
    Q_OBJECT

public:
    enum class ProtocolMode {
        SingleTurn = 0,   // Mode 2 (0x01): auto-report single-turn value
        MultiTurn  = 1,   // Mode 3 (0x04): auto-report virtual multi-turn value
        Velocity   = 2    // Mode 4 (0x05): auto-report angular velocity
    };

    explicit EncoderProactiveListener(QSerialPort* serialPort,
                                       ProtocolMode mode,
                                       uint16_t resolution = 4096,
                                       QObject* parent = nullptr);
    ~EncoderProactiveListener() override;

    void start();
    void stop();

    // Data accessors (thread-safe, atomic)
    double latestAngle() const;       // Valid in SingleTurn mode
    double latestMultiTurn() const;   // Valid in MultiTurn mode (total degrees)
    double latestVelocity() const;    // Valid in Velocity mode (RPM)
    bool isValid() const;
    ProtocolMode mode() const;
    void setResolution(uint16_t resolution);

signals:
    void dataReceived(double value);
    void errorOccurred(const QString& error);

private slots:
    void onReadyRead();

private:
    // Frame parsing per mode
    void parseSingleTurnFrame();
    void parseMultiTurnFrame();
    void parseVelocityFrame();

    QSerialPort* m_serialPort;
    ProtocolMode m_mode;
    QByteArray m_buffer;
    uint16_t m_resolution;

    std::atomic<bool> m_running{false};
    std::atomic<bool> m_valid{false};

    // Cached values (atomic for thread safety with Poller)
    std::atomic<double> m_latestAngle{0.0};
    std::atomic<double> m_latestMultiTurn{0.0};
    std::atomic<double> m_latestVelocity{0.0};
};

} // namespace Devices
} // namespace Infrastructure

#endif // ENCODERPROACTIVELISTENER_H
