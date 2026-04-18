#ifndef DYN200PROACTIVELISTENER_H
#define DYN200PROACTIVELISTENER_H

#include <QObject>
#include <QSerialPort>
#include <QByteArray>

namespace Infrastructure {
namespace Devices {

/**
 * @brief Listener for DYN200 torque sensor proactive upload protocols
 * 
 * Supports three protocol modes:
 * - Hex6Byte: 6-byte HEX format (D1D2=torque, D3D4=speed, D5D6=CRC)
 * - Hex8Byte: 8-byte HEX format (D1D2D3=torque, D4D5D6=speed, D7D8=CRC)
 * - Ascii: ASCII format ([+-]X.XXXX\r, torque only)
 */
class Dyn200ProactiveListener : public QObject {
    Q_OBJECT

public:
    enum ProtocolMode {
        Hex6Byte = 0,  // Communication mode 0: 6-byte HEX
        Ascii = 2,     // Communication mode 2: ASCII
        Hex8Byte = 3   // Communication mode 3: 8-byte HEX
    };

    explicit Dyn200ProactiveListener(ProtocolMode mode, QObject* parent = nullptr);
    ~Dyn200ProactiveListener() override;

    /**
     * @brief Start listening on the given serial port
     * @param serialPort Serial port to listen on (must be already open)
     */
    void start(QSerialPort* serialPort);

    /**
     * @brief Stop listening
     */
    void stop();

    /**
     * @brief Get the latest torque value
     */
    double latestTorque() const { return m_latestTorque; }

    /**
     * @brief Get the latest speed value
     */
    double latestSpeed() const { return m_latestSpeed; }

    /**
     * @brief Check if speed is valid (ASCII mode doesn't provide speed)
     */
    bool isSpeedValid() const { return m_speedValid; }

signals:
    /**
     * @brief Emitted when new data is received and parsed
     * @param torqueNm Torque in N·m
     * @param speedRpm Speed in RPM
     * @param speedValid true if speed is valid
     */
    void dataReceived(double torqueNm, double speedRpm, bool speedValid);

    /**
     * @brief Emitted when an error occurs
     */
    void errorOccurred(const QString& error);

private slots:
    void onReadyRead();

private:
    void parseHex6ByteFrame();
    void parseHex8ByteFrame();
    void parseAsciiFrame();
    uint16_t calculateCRC16(const QByteArray& data) const;

    QSerialPort* m_serialPort;
    ProtocolMode m_mode;
    QByteArray m_buffer;
    
    double m_latestTorque;
    double m_latestSpeed;
    bool m_speedValid;
};

} // namespace Devices
} // namespace Infrastructure

#endif // DYN200PROACTIVELISTENER_H
