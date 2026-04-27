#ifndef ACQUISITIONSCHEDULER_H
#define ACQUISITIONSCHEDULER_H

#include <QObject>
#include <memory>
#include "TelemetryBuffer.h"
#include "DevicePoller.h"
#include "../devices/IMotorDriveDevice.h"
#include "../devices/ITorqueSensorDevice.h"
#include "../devices/IEncoderDevice.h"
#include "../devices/IBrakePowerDevice.h"
#include "../../domain/TelemetrySnapshot.h"

namespace Infrastructure {
namespace Acquisition {

class AcquisitionScheduler : public QObject {
    Q_OBJECT

public:
    explicit AcquisitionScheduler(QObject* parent = nullptr);
    ~AcquisitionScheduler();

    void setMotorDevice(Devices::IMotorDriveDevice* device, int pollIntervalUs);
    void setTorqueDevice(Devices::ITorqueSensorDevice* device, int pollIntervalUs);
    void setEncoderDevice(Devices::IEncoderDevice* device, int pollIntervalUs);
    void setBrakeDevice(Devices::IBrakePowerDevice* device, int channel, int pollIntervalUs);

    bool start();
    void stop();
    bool isRunning() const;

    void setEncoderPollInterval(int intervalMs);
    int getEncoderPollInterval() const;

    TelemetryBuffer* buffer() { return &m_buffer; }
    const TelemetryBuffer* buffer() const { return &m_buffer; }

    Domain::TelemetrySnapshot snapshot() const;
    AcquisitionStats stats() const;

signals:
    void started();
    void stopped();
    void errorOccurred(const QString& error);

private:
    TelemetryBuffer m_buffer;

    std::unique_ptr<MotorPoller> m_motorPoller;
    std::unique_ptr<TorquePoller> m_torquePoller;
    std::unique_ptr<EncoderPoller> m_encoderPoller;
    std::unique_ptr<BrakePoller> m_brakePoller;

    Devices::IMotorDriveDevice* m_motorDevice = nullptr;
    Devices::ITorqueSensorDevice* m_torqueDevice = nullptr;
    Devices::IEncoderDevice* m_encoderDevice = nullptr;
    Devices::IBrakePowerDevice* m_brakeDevice = nullptr;

    int m_motorIntervalUs = 5000;
    int m_torqueIntervalUs = 5000;
    int m_encoderIntervalUs = 5000;
    int m_brakeIntervalUs = 5000;
    int m_brakeChannel = 1;
    bool m_running = false;
};

} // namespace Acquisition
} // namespace Infrastructure

#endif // ACQUISITIONSCHEDULER_H
