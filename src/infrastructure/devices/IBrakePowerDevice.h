#ifndef IBRAKEPOWERDEVICE_H
#define IBRAKEPOWERDEVICE_H

#include <QObject>
#include <QString>

namespace Infrastructure {
namespace Devices {

/**
 * @brief Abstract interface for brake power supply device
 * 
 * Controls magnetic powder brake through programmable power supply.
 */
class IBrakePowerDevice : public QObject {
    Q_OBJECT

public:
    explicit IBrakePowerDevice(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~IBrakePowerDevice() = default;

    /**
     * @brief Initialize device and verify communication
     */
    virtual bool initialize() = 0;

    /**
     * @brief Set output current in Amperes
     * @param channel Channel number (1 or 2)
     * @param currentA Current in Amperes
     */
    virtual bool setCurrent(int channel, double currentA) = 0;

    /**
     * @brief Enable or disable output
     * @param channel Channel number (1 or 2)
     * @param enable true to enable, false to disable
     */
    virtual bool setOutputEnable(int channel, bool enable) = 0;

    /**
     * @brief Read actual output current
     * @param channel Channel number (1 or 2)
     * @param currentA Output current in Amperes
     */
    virtual bool readCurrent(int channel, double& currentA) = 0;

    /**
     * @brief Get last error message
     */
    virtual QString lastError() const = 0;

signals:
    void errorOccurred(const QString& error);
};

} // namespace Devices
} // namespace Infrastructure

#endif // IBRAKEPOWERDEVICE_H
