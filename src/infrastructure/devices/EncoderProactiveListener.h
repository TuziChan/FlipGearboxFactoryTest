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
 * - Mode 2: Auto-report single-turn value
 * - Mode 3: Auto-report virtual multi-turn value
 * - Mode 4: Auto-report angular velocity
 */
class EncoderProactiveListener : public QObject {
    Q_OBJECT

public:
    explicit EncoderProactiveListener(QSerialPort* serialPort, QObject* parent = nullptr);
    ~EncoderProactiveListener() override;

    void start();
    void stop();
    double latestAngle() const;
    bool isValid() const;

signals:
    void dataReceived(double angleDeg);
    void errorOccurred(const QString& error);

private slots:
    void onReadyRead();

private:
    QSerialPort* m_serialPort;
    QByteArray m_buffer;
    std::atomic<double> m_latestAngle{0.0};
    std::atomic<bool> m_valid{false};
};

} // namespace Devices
} // namespace Infrastructure

#endif // ENCODERPROACTIVELISTENER_H
