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
     * @brief Get last error message
     */
    virtual QString lastError() const = 0;

signals:
    void errorOccurred(const QString& error);
};

} // namespace Devices
} // namespace Infrastructure

#endif // IENCODERDEVICE_H
