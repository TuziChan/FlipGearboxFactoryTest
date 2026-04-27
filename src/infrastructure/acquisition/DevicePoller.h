#ifndef DEVICEPOLLER_H
#define DEVICEPOLLER_H

#include <QThread>
#include <atomic>
#include "../bus/IBusController.h"
#include "../devices/IMotorDriveDevice.h"
#include "../devices/ITorqueSensorDevice.h"
#include "../devices/IEncoderDevice.h"
#include "../devices/IBrakePowerDevice.h"
#include "TelemetryBuffer.h"

namespace Infrastructure {
namespace Acquisition {

class MotorPoller : public QThread {
    Q_OBJECT
public:
    MotorPoller(Devices::IMotorDriveDevice* device, MotorTelemetry* buffer, int intervalUs, QObject* parent = nullptr)
        : QThread(parent), m_device(device), m_buffer(buffer), m_intervalUs(intervalUs) {}

    void stop() { m_running.store(false); }

signals:
    void errorOccurred(const QString& error);

protected:
    void run() override {
        m_running.store(true);
        while (m_running.load()) {
            double currentA = 0.0;
            bool ai1 = true;

            bool ok = true;
            if (!m_device->readCurrent(currentA)) ok = false;
            if (!m_device->readAI1Level(ai1)) ok = false;

            uint64_t ts = TelemetryBuffer::nowNs();
            m_buffer->timestampNs.store(ts);

            if (ok) {
                m_buffer->currentA.store(currentA);
                m_buffer->ai1Level.store(ai1);
                m_buffer->valid.store(true);
                m_buffer->successCount.fetch_add(1);
            } else {
                m_buffer->valid.store(false);
                m_buffer->errorCount.fetch_add(1);
            }

            usleep(m_intervalUs);
        }
    }

private:
    Devices::IMotorDriveDevice* m_device;
    MotorTelemetry* m_buffer;
    int m_intervalUs;
    std::atomic<bool> m_running{false};

    void usleep(int us) {
        QThread::usleep(std::max(us, 100));
    }
};

class TorquePoller : public QThread {
    Q_OBJECT
public:
    TorquePoller(Devices::ITorqueSensorDevice* device, TorqueTelemetry* buffer, int intervalUs, QObject* parent = nullptr)
        : QThread(parent), m_device(device), m_buffer(buffer), m_intervalUs(intervalUs) {}

    void stop() { m_running.store(false); }

signals:
    void errorOccurred(const QString& error);

protected:
    void run() override {
        m_running.store(true);
        while (m_running.load()) {
            double torqueNm = 0.0, speedRpm = 0.0, powerW = 0.0;
            bool ok = m_device->readAll(torqueNm, speedRpm, powerW);

            uint64_t ts = TelemetryBuffer::nowNs();
            m_buffer->timestampNs.store(ts);

            if (ok) {
                m_buffer->torqueNm.store(torqueNm);
                m_buffer->speedRpm.store(speedRpm);
                m_buffer->powerW.store(powerW);
                m_buffer->valid.store(true);
                m_buffer->successCount.fetch_add(1);
            } else {
                m_buffer->valid.store(false);
                m_buffer->errorCount.fetch_add(1);
            }

            usleep(m_intervalUs);
        }
    }

private:
    Devices::ITorqueSensorDevice* m_device;
    TorqueTelemetry* m_buffer;
    int m_intervalUs;
    std::atomic<bool> m_running{false};

    void usleep(int us) {
        QThread::usleep(std::max(us, 100));
    }
};

class EncoderPoller : public QThread {
    Q_OBJECT
public:
    EncoderPoller(Devices::IEncoderDevice* device, EncoderTelemetry* buffer, int intervalUs, QObject* parent = nullptr)
        : QThread(parent), m_device(device), m_buffer(buffer), m_intervalUs(intervalUs) {}

    void stop() { m_running.store(false); }

signals:
    void errorOccurred(const QString& error);

protected:
    void run() override {
        m_running.store(true);
        while (m_running.load()) {
            double angleDeg = 0.0;
            double totalAngleDeg = 0.0;
            double velocityRpm = 0.0;

            bool angleOk = m_device->readAngle(angleDeg);
            m_device->readVirtualMultiTurn(totalAngleDeg);
            m_device->readAngularVelocity(velocityRpm);

            uint64_t ts = TelemetryBuffer::nowNs();
            m_buffer->timestampNs.store(ts);

            if (angleOk) {
                m_buffer->angleDeg.store(angleDeg);
                m_buffer->totalAngleDeg.store(totalAngleDeg);
                m_buffer->velocityRpm.store(velocityRpm);
                m_buffer->valid.store(true);
                m_buffer->successCount.fetch_add(1);
            } else {
                m_buffer->valid.store(false);
                m_buffer->errorCount.fetch_add(1);
            }

            usleep(m_intervalUs);
        }
    }

private:
    Devices::IEncoderDevice* m_device;
    EncoderTelemetry* m_buffer;
    int m_intervalUs;
    std::atomic<bool> m_running{false};

    void usleep(int us) {
        QThread::usleep(std::max(us, 100));
    }
};

class BrakePoller : public QThread {
    Q_OBJECT
public:
    BrakePoller(Devices::IBrakePowerDevice* device, int channel, BrakeTelemetry* buffer, int intervalUs, QObject* parent = nullptr)
        : QThread(parent), m_device(device), m_channel(channel), m_buffer(buffer), m_intervalUs(intervalUs) {}

    void stop() { m_running.store(false); }

signals:
    void errorOccurred(const QString& error);

protected:
    void run() override {
        m_running.store(true);
        while (m_running.load()) {
            double currentA = 0.0;
            double voltageV = 0.0;
            double powerW = 0.0;

            bool ok = true;
            if (!m_device->readCurrent(m_channel, currentA)) ok = false;
            if (!m_device->readVoltage(m_channel, voltageV)) ok = false;
            if (!m_device->readPower(m_channel, powerW)) ok = false;

            uint64_t ts = TelemetryBuffer::nowNs();
            m_buffer->timestampNs.store(ts);

            if (ok) {
                m_buffer->currentA.store(currentA);
                m_buffer->voltageV.store(voltageV);
                m_buffer->powerW.store(powerW);
                m_buffer->valid.store(true);
                m_buffer->successCount.fetch_add(1);
            } else {
                m_buffer->valid.store(false);
                m_buffer->errorCount.fetch_add(1);
            }

            usleep(m_intervalUs);
        }
    }

private:
    Devices::IBrakePowerDevice* m_device;
    int m_channel;
    BrakeTelemetry* m_buffer;
    int m_intervalUs;
    std::atomic<bool> m_running{false};

    void usleep(int us) {
        QThread::usleep(std::max(us, 100));
    }
};

} // namespace Acquisition
} // namespace Infrastructure

#endif // DEVICEPOLLER_H
