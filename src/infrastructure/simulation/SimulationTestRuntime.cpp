#include "SimulationTestRuntime.h"
#include <QDebug>

namespace Infrastructure {
namespace Simulation {

SimulationTestRuntime::SimulationTestRuntime(QObject* parent)
    : QObject(parent)
    , m_brakeChannel(1)
    , m_initialized(false)
{
    // Create shared simulation context
    m_context = std::make_shared<SimulationContext>();
    
    // Create test helper
    m_helper = std::make_unique<SimulationTestHelper>(m_context.get(), this);
    
    // Create simulated devices
    m_motor = std::make_unique<SimulatedMotorDevice>(m_context.get(), this);
    m_torque = std::make_unique<SimulatedTorqueDevice>(m_context.get(), this);
    m_encoder = std::make_unique<SimulatedEncoderDevice>(m_context.get(), this);
    m_brake = std::make_unique<SimulatedBrakeDevice>(m_context.get(), this);
    
    // Create test engine
    m_testEngine = std::make_unique<Domain::GearboxTestEngine>(this);
    m_testEngine->setBrakeChannel(m_brakeChannel);
    m_testEngine->setDevices(
        m_motor.get(),
        m_torque.get(),
        m_encoder.get(),
        m_brake.get()
    );
    
    qDebug() << "SimulationTestRuntime created";
}

SimulationTestRuntime::~SimulationTestRuntime() {
    qDebug() << "SimulationTestRuntime destroyed";
}

// ========== Lifecycle ==========

bool SimulationTestRuntime::initialize() {
    if (m_initialized) {
        qWarning() << "SimulationTestRuntime already initialized";
        return true;
    }
    
    // Initialize all devices
    if (!m_motor->initialize()) {
        qCritical() << "Failed to initialize motor device";
        return false;
    }
    
    if (!m_torque->initialize()) {
        qCritical() << "Failed to initialize torque device";
        return false;
    }
    
    if (!m_encoder->initialize()) {
        qCritical() << "Failed to initialize encoder device";
        return false;
    }
    
    if (!m_brake->initialize()) {
        qCritical() << "Failed to initialize brake device";
        return false;
    }
    
    m_initialized = true;
    qDebug() << "SimulationTestRuntime initialized successfully";
    return true;
}

void SimulationTestRuntime::reset() {
    m_context->reset();
    m_helper->reset();
    
    if (m_testEngine) {
        m_testEngine->reset();
    }
    
    qDebug() << "SimulationTestRuntime reset";
}

// ========== Configuration ==========

void SimulationTestRuntime::setBrakeChannel(int channel) {
    m_brakeChannel = channel;
    if (m_testEngine) {
        m_testEngine->setBrakeChannel(channel);
    }
}

// ========== Convenience Methods ==========

bool SimulationTestRuntime::startTest(const QString& serialNumber) {
    if (!m_initialized) {
        qWarning() << "Cannot start test: runtime not initialized";
        return false;
    }
    
    return m_testEngine->startTest(serialNumber);
}

Domain::TestRunState SimulationTestRuntime::currentState() const {
    if (!m_testEngine) {
        return Domain::TestRunState();
    }
    return m_testEngine->currentState();
}

Domain::TestResults SimulationTestRuntime::results() const {
    if (!m_testEngine) {
        return Domain::TestResults();
    }
    return m_testEngine->currentState().results;
}

} // namespace Simulation
} // namespace Infrastructure
