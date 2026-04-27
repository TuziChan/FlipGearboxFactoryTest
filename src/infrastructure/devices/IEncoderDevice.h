#ifndef IENCODERDEVICE_H
#define IENCODERDEVICE_H

#include <QObject>
#include <QString>

namespace Infrastructure {
namespace Devices {

/**
 * @brief Abstract interface for encoder device
 * 
 * Provides absolute angle reading and zero-point setting.
 */
class IEncoderDevice : public QObject {
    Q_OBJECT

public:
    explicit IEncoderDevice(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~IEncoderDevice() = default;

    /**
     * @brief Initialize device and verify communication
     */
    virtual bool initialize() = 0;

    /**
     * @brief Read absolute angle in degrees (0-360)
     */
    virtual bool readAngle(double& angleDeg) = 0;

    /**
     * @brief Read virtual multi-turn angle in degrees (accumulated total rotation)
     */
    virtual bool readVirtualMultiTurn(double& totalAngleDeg) = 0;

    /**
     * @brief Read angular velocity in RPM
     */
    virtual bool readAngularVelocity(double& velocityRpm) = 0;

    /**
     * @brief Set current position as zero point
     */
    virtual bool setZeroPoint() = 0;

    /**
     * @brief Set encoder resolution (pulses per revolution)
     */
    virtual bool setResolution(uint16_t resolution) = 0;

    /**
     * @brief Get current resolution
     */
    virtual uint16_t getResolution() const = 0;

    /**
     * @brief Get current communication mode
     */
    virtual int getCommunicationMode() const = 0;

    /**
     * @brief Set auto-report mode at runtime
     * @param mode 0x00=off, 0x01=single-turn, 0x04=multi-turn, 0x05=velocity
     * @param intervalMs Report interval in milliseconds
     */
    virtual bool setAutoReportMode(uint16_t mode, int intervalMs) = 0;

    /**
     * @brief Get last error message
     */
    virtual QString lastError() const = 0;

signals:
    void errorOccurred(const QString& error);
};

} // namespace Devices
} // namespace Infrastructure

#endif // IENCODERDEVICE_H
