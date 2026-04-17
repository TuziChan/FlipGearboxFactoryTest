#ifndef ITORQUESENSORDEVICE_H
#define ITORQUESENSORDEVICE_H

#include <QObject>
#include <QString>

namespace Infrastructure {
namespace Devices {

/**
 * @brief Abstract interface for torque sensor device (DYN200)
 * 
 * Provides torque, speed, and power readings.
 */
class ITorqueSensorDevice : public QObject {
    Q_OBJECT

public:
    explicit ITorqueSensorDevice(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~ITorqueSensorDevice() = default;

    /**
     * @brief Initialize device and verify communication
     */
    virtual bool initialize() = 0;

    /**
     * @brief Read torque in N·m
     */
    virtual bool readTorque(double& torqueNm) = 0;

    /**
     * @brief Read speed in RPM
     */
    virtual bool readSpeed(double& speedRpm) = 0;

    /**
     * @brief Read power in Watts
     */
    virtual bool readPower(double& powerW) = 0;

    /**
     * @brief Read all telemetry at once (more efficient)
     */
    virtual bool readAll(double& torqueNm, double& speedRpm, double& powerW) = 0;

    /**
     * @brief Get last error message
     */
    virtual QString lastError() const = 0;

signals:
    void errorOccurred(const QString& error);
};

} // namespace Devices
} // namespace Infrastructure

#endif // ITORQUESENSORDEVICE_H
