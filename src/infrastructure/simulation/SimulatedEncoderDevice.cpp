#include "SimulatedEncoderDevice.h"
#include <QDebug>

namespace Infrastructure {
namespace Simulation {

SimulatedEncoderDevice::SimulatedEncoderDevice(SimulationContext* context, QObject* parent)
    : IEncoderDevice(parent)
    , m_context(context)
{
}

bool SimulatedEncoderDevice::initialize() {
    return true;
}

bool SimulatedEncoderDevice::readAngle(double& angleDeg) {
    if (!m_context) return false;

    m_context->incrementTickCount();

    // Get angle from context (already wrapped to 0-360)
    angleDeg = m_context->encoderAngleDeg();
    return true;
}

bool SimulatedEncoderDevice::readVirtualMultiTurn(double& totalAngleDeg) {
    if (!m_context) return false;

    m_context->incrementTickCount();

    // Get total accumulated angle
    totalAngleDeg = m_context->encoderTotalAngleDeg();
    return true;
}

bool SimulatedEncoderDevice::readAngularVelocity(double& velocityRpm) {
    if (!m_context) return false;

    m_context->incrementTickCount();

    // Get velocity from context
    velocityRpm = m_context->encoderAngularVelocityRpm();
    return true;
}

bool SimulatedEncoderDevice::setZeroPoint() {
    if (!m_context) return false;

    // In simulation, the encoder zero point is fixed at installation time.
    // Real absolute encoders have a physically defined zero that doesn't change at runtime.
    // setZeroPoint() is kept as a no-op acknowledgement for protocol compatibility.
    qDebug("SimulatedEncoderDevice::setZeroPoint() - no-op, encoder zero is fixed at installation");
    return true;
}

QString SimulatedEncoderDevice::lastError() const {
    return QString();
}

} // namespace Simulation
} // namespace Infrastructure
