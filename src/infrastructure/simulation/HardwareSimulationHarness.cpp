#include "HardwareSimulationHarness.h"
#include <QDebug>

namespace Infrastructure {
namespace Simulation {

HardwareSimulationHarness::HardwareSimulationHarness(QObject* parent)
    : QObject(parent)
    , m_context(std::make_shared<EnhancedSimulationContext>())
    , m_busNormal(std::make_unique<SimulatedBusController>())
    , m_busFaulty(std::make_unique<SimulatedBusControllerWithFaults>())
{
    createDevices();
}

HardwareSimulationHarness::~HardwareSimulationHarness() {
    shutdown();
}

void HardwareSimulationHarness::createDevices() {
    m_motor = std::make_unique<SimulatedMotorDevice>(m_context.get(), this);
    m_torque = std::make_unique<SimulatedTorqueDevice>(m_context.get(), this);
    m_encoder = std::make_unique<SimulatedEncoderDevice>(m_context.get(), this);
    m_brake = std::make_unique<SimulatedBrakeDevice>(m_context.get(), this);
}

bool HardwareSimulationHarness::initialize() {
    if (m_initialized) {
        return true;
    }

    qDebug() << "[HardwareSimulationHarness] Initializing simulated hardware...";

    // Open all bus controllers
    m_busNormal->open("SIM_BUS", 9600, 1000, "None", 1);
    m_busFaulty->open("SIM_FAULTY_BUS", 9600, 1000, "None", 1);

    // Initialize all devices
    bool motorOk = m_motor->initialize();
    bool torqueOk = m_torque->initialize();
    bool encoderOk = m_encoder->initialize();
    bool brakeOk = m_brake->initialize();

    if (!motorOk || !torqueOk || !encoderOk || !brakeOk) {
        qWarning() << "[HardwareSimulationHarness] Some devices failed to initialize";
    }

    m_initialized = true;
    emit initialized();
    return true;
}

void HardwareSimulationHarness::shutdown() {
    if (!m_initialized) {
        return;
    }

    qDebug() << "[HardwareSimulationHarness] Shutting down...";

    m_busNormal->close();
    m_busFaulty->close();

    m_initialized = false;
    emit shutdownComplete();
}

bool HardwareSimulationHarness::isRunning() const {
    return m_initialized;
}

void HardwareSimulationHarness::resetToDefaults() {
    m_context->reset();
    m_context->resetFaults();
    m_busFaulty->setFaultMode(SimulatedBusControllerWithFaults::FaultMode::None);
    m_busFaulty->resetStatistics();
    emit stateChanged();
}

void HardwareSimulationHarness::setMotorState(SimulationContext::MotorDirection direction, double dutyCycle) {
    m_context->setMotorDirection(direction);
    m_context->setMotorDutyCycle(dutyCycle);
    emit stateChanged();
}

void HardwareSimulationHarness::setBrakeOutput(bool enabled, double currentA) {
    m_context->setBrakeOutputEnabled(enabled);
    m_context->setBrakeCurrent(currentA);
    emit stateChanged();
}

void HardwareSimulationHarness::setEncoderZeroOffset(double offsetDeg) {
    m_context->setEncoderZeroOffset(offsetDeg);
    emit stateChanged();
}

void HardwareSimulationHarness::injectBusFault(int busIndex, SimulatedBusControllerWithFaults::FaultMode mode) {
    Q_UNUSED(busIndex)
    m_busFaulty->setFaultMode(mode);
    emit faultInjected(QString::fromLatin1("BusFault"), QString::number(static_cast<int>(mode)));
}

void HardwareSimulationHarness::clearBusFaults() {
    m_busFaulty->setFaultMode(SimulatedBusControllerWithFaults::FaultMode::None);
    m_busFaulty->resetStatistics();
}

void HardwareSimulationHarness::injectSensorFault(EnhancedSimulationContext::FaultType encoderFault,
                                                   EnhancedSimulationContext::FaultType torqueFault,
                                                   EnhancedSimulationContext::FaultType speedFault) {
    m_context->setEncoderFault(encoderFault);
    m_context->setTorqueFault(torqueFault);
    m_context->setSpeedFault(speedFault);
    emit faultInjected(QString::fromLatin1("SensorFault"), QStringLiteral("encoder/torque/speed"));
}

void HardwareSimulationHarness::clearSensorFaults() {
    m_context->resetFaults();
}

void HardwareSimulationHarness::setMagnetPositions(const QVector<double>& positions) {
    m_context->setMagnetPositions(positions);
}

QVector<double> HardwareSimulationHarness::magnetPositions() const {
    return m_context->magnetPositions();
}

Devices::IMotorDriveDevice* HardwareSimulationHarness::motorDevice() const {
    return m_motor.get();
}

Devices::ITorqueSensorDevice* HardwareSimulationHarness::torqueDevice() const {
    return m_torque.get();
}

Devices::IEncoderDevice* HardwareSimulationHarness::encoderDevice() const {
    return m_encoder.get();
}

Devices::IBrakePowerDevice* HardwareSimulationHarness::brakeDevice() const {
    return m_brake.get();
}

Bus::IBusController* HardwareSimulationHarness::busController(int index) const {
    Q_UNUSED(index)
    return m_useFaultyBus ? static_cast<Bus::IBusController*>(m_busFaulty.get())
                          : static_cast<Bus::IBusController*>(m_busNormal.get());
}

SimulationContext* HardwareSimulationHarness::simulationContext() const {
    return m_context.get();
}

EnhancedSimulationContext* HardwareSimulationHarness::enhancedContext() const {
    return m_context.get();
}

HardwareSimulationHarness::HarnessStats HardwareSimulationHarness::stats() const {
    HarnessStats s;
    s.busFailureCount = m_busFaulty->failureCount();
    s.busSuccessCount = m_busFaulty->successCount();
    s.busTimeoutCount = m_busFaulty->timeoutCount();
    s.busCrcErrorCount = m_busFaulty->crcErrorCount();
    s.totalTicks = m_context->tickCount();
    s.motorInitialized = m_motor != nullptr;
    s.torqueInitialized = m_torque != nullptr;
    s.encoderInitialized = m_encoder != nullptr;
    s.brakeInitialized = m_brake != nullptr;
    return s;
}

double HardwareSimulationHarness::currentEncoderAngle() const {
    return m_context->encoderAngleDeg();
}

double HardwareSimulationHarness::currentMotorSpeedRpm() const {
    return m_context->encoderAngularVelocityRpm();
}

double HardwareSimulationHarness::currentTorqueNm() const {
    return m_context->getTorqueWithFaults();
}

} // namespace Simulation
} // namespace Infrastructure
