#ifndef IMOTORDRIVEDEVICE_H
#define IMOTORDRIVEDEVICE_H

#include <QObject>
#include <QString>

namespace Infrastructure {
namespace Devices {

/**
 * @brief Abstract interface for motor drive device (AQMD)
 * 
 * Provides motor control, current reading, and magnet event detection.
 */
class IMotorDriveDevice : public QObject {
    Q_OBJECT

public:
    enum class Direction {
        Forward,
        Reverse,
        Brake
    };

    explicit IMotorDriveDevice(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~IMotorDriveDevice() = default;

    /**
     * @brief Initialize device and verify communication
     */
    virtual bool initialize() = 0;

    /**
     * @brief Set motor direction and duty cycle
     * @param direction Motor direction
     * @param dutyCyclePercent Duty cycle 0.0 - 100.0%
     */
    virtual bool setMotor(Direction direction, double dutyCyclePercent) = 0;

    /**
     * @brief Stop motor with brake
     */
    virtual bool brake() = 0;

    /**
     * @brief Stop motor naturally (coast)
     */
    virtual bool coast() = 0;

    /**
     * @brief Read motor current in Amperes
     */
    virtual bool readCurrent(double& currentA) = 0;

    /**
     * @brief Read AI1 digital level (for magnet detection)
     * @param level Output: true = high, false = low
     */
    virtual bool readAI1Level(bool& level) = 0;

    /**
     * @brief Get last error message
     */
    virtual QString lastError() const = 0;

signals:
    void errorOccurred(const QString& error);
};

} // namespace Devices
} // namespace Infrastructure

#endif // IMOTORDRIVEDEVICE_H
