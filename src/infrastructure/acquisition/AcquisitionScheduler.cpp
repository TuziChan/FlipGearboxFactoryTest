#include "AcquisitionScheduler.h"
#include <QDebug>

namespace Infrastructure {
namespace Acquisition {

AcquisitionScheduler::AcquisitionScheduler(QObject* parent)
    : QObject(parent)
    , m_buffer()
    , m_running(false)
{
}

AcquisitionScheduler::~AcquisitionScheduler() {
    stop();
}

void AcquisitionScheduler::setMotorDevice(Devices::IMotorDriveDevice* device, int pollIntervalUs) {
    m_motorDevice = device;
    m_motorIntervalUs = pollIntervalUs;
}

void AcquisitionScheduler::setTorqueDevice(Devices::ITorqueSensorDevice* device, int pollIntervalUs) {
    m_torqueDevice = device;
    m_torqueIntervalUs = pollIntervalUs;
}

void AcquisitionScheduler::setEncoderDevice(Devices::IEncoderDevice* device, int pollIntervalUs) {
    m_encoderDevice = device;
    m_encoderIntervalUs = pollIntervalUs;
}

void AcquisitionScheduler::setBrakeDevice(Devices::IBrakePowerDevice* device, int channel, int pollIntervalUs) {
    m_brakeDevice = device;
    m_brakeChannel = channel;
    m_brakeIntervalUs = pollIntervalUs;
}

bool AcquisitionScheduler::start() {
    if (m_running) {
        return true;
    }

    if (m_motorDevice) {
        m_motorPoller = std::make_unique<MotorPoller>(m_motorDevice, &m_buffer.motor, m_motorIntervalUs, this);
        m_motorPoller->start();
        qDebug() << "Motor poller started, interval:" << m_motorIntervalUs << "us";
    }

    if (m_torqueDevice) {
        m_torquePoller = std::make_unique<TorquePoller>(m_torqueDevice, &m_buffer.torque, m_torqueIntervalUs, this);
        m_torquePoller->start();
        qDebug() << "Torque poller started, interval:" << m_torqueIntervalUs << "us";
    }

    if (m_encoderDevice) {
        m_encoderPoller = std::make_unique<EncoderPoller>(m_encoderDevice, &m_buffer.encoder, m_encoderIntervalUs, this);
        m_encoderPoller->start();
        qDebug() << "Encoder poller started, interval:" << m_encoderIntervalUs << "us";
    }

    if (m_brakeDevice) {
        m_brakePoller = std::make_unique<BrakePoller>(m_brakeDevice, m_brakeChannel, &m_buffer.brake, m_brakeIntervalUs, this);
        m_brakePoller->start();
        qDebug() << "Brake poller started, interval:" << m_brakeIntervalUs << "us, channel:" << m_brakeChannel;
    }

    m_running = true;
    emit started();
    qDebug() << "Acquisition scheduler started";
    return true;
}

void AcquisitionScheduler::stop() {
    if (!m_running) {
        return;
    }

    m_running = false;

    if (m_motorPoller) {
        m_motorPoller->stop();
        m_motorPoller.reset();
    }

    if (m_torquePoller) {
        m_torquePoller->stop();
        m_torquePoller.reset();
    }

    if (m_encoderPoller) {
        m_encoderPoller->stop();
        m_encoderPoller.reset();
    }

    if (m_brakePoller) {
        m_brakePoller->stop();
        m_brakePoller.reset();
    }

    emit stopped();
    qDebug() << "Acquisition scheduler stopped";
}

bool AcquisitionScheduler::isRunning() const {
    return m_running;
}

Domain::TelemetrySnapshot AcquisitionScheduler::snapshot() const {
    Domain::TelemetrySnapshot snap;

    // Check data validity to avoid reading uninitialized values
    if (!m_buffer.motor.valid.load() ||
        !m_buffer.torque.valid.load() ||
        !m_buffer.encoder.valid.load() ||
        !m_buffer.brake.valid.load()) {
        qWarning() << "Telemetry buffer not fully initialized, some data may be invalid";
    }

    snap.motorCurrentA = m_buffer.motor.currentA.load();
    snap.aqmdAi1Level = m_buffer.motor.ai1Level.load();
    snap.motorOnline = m_buffer.motor.valid.load();

    snap.dynTorqueNm = m_buffer.torque.torqueNm.load();
    snap.dynSpeedRpm = m_buffer.torque.speedRpm.load();
    snap.dynPowerW = m_buffer.torque.powerW.load();
    snap.torqueOnline = m_buffer.torque.valid.load();

    snap.encoderAngleDeg = m_buffer.encoder.angleDeg.load();
    snap.encoderTotalAngleDeg = m_buffer.encoder.totalAngleDeg.load();
    snap.encoderVelocityRpm = m_buffer.encoder.velocityRpm.load();
    snap.encoderOnline = m_buffer.encoder.valid.load();

    snap.brakeCurrentA = m_buffer.brake.currentA.load();
    snap.brakeVoltageV = m_buffer.brake.voltageV.load();
    snap.brakePowerW = m_buffer.brake.powerW.load();
    snap.brakeOnline = m_buffer.brake.valid.load();

    return snap;
}

AcquisitionStats AcquisitionScheduler::stats() const {
    AcquisitionStats s;

    auto calcHz = [](int successCount, int intervalUs) -> int {
        if (intervalUs <= 0 || successCount <= 0) return 0;
        return static_cast<int>(1000000.0 / intervalUs);
    };

    s.motorPollHz = calcHz(m_buffer.motor.successCount.load(), m_motorIntervalUs);
    s.torquePollHz = calcHz(m_buffer.torque.successCount.load(), m_torqueIntervalUs);
    s.encoderPollHz = calcHz(m_buffer.encoder.successCount.load(), m_encoderIntervalUs);
    s.brakePollHz = calcHz(m_buffer.brake.successCount.load(), m_brakeIntervalUs);

    s.motorErrors = m_buffer.motor.errorCount.load();
    s.torqueErrors = m_buffer.torque.errorCount.load();
    s.encoderErrors = m_buffer.encoder.errorCount.load();
    s.brakeErrors = m_buffer.brake.errorCount.load();

    return s;
}

void AcquisitionScheduler::setEncoderPollInterval(int intervalMs) {
    m_encoderIntervalUs = intervalMs * 1000;

    // Restart encoder poller if running
    if (m_running && m_encoderPoller) {
        m_encoderPoller->stop();
        m_encoderPoller.reset();

        if (m_encoderDevice) {
            m_encoderPoller = std::make_unique<EncoderPoller>(
                m_encoderDevice,
                &m_buffer.encoder,
                m_encoderIntervalUs
            );
            m_encoderPoller->start();
        }
    }
}

int AcquisitionScheduler::getEncoderPollInterval() const {
    return m_encoderIntervalUs / 1000;
}

} // namespace Acquisition
} // namespace Infrastructure
