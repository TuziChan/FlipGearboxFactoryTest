#include "SimulatedEncoderDevice.h"

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

    m_context->advanceTick();

    // Get angle from context (already wrapped to 0-360)
    angleDeg = m_context->encoderAngleDeg();
    return true;
}

bool SimulatedEncoderDevice::readVirtualMultiTurn(double& totalAngleDeg) {
    if (!m_context) return false;

    m_context->advanceTick();

    // Get total accumulated angle
    totalAngleDeg = m_context->encoderTotalAngleDeg();
    return true;
}

bool SimulatedEncoderDevice::readAngularVelocity(double& velocityRpm) {
    if (!m_context) return false;

    m_context->advanceTick();

    // Get velocity from context
    velocityRpm = m_context->encoderAngularVelocityRpm();
    return true;
}

bool SimulatedEncoderDevice::setZeroPoint() {
    if (!m_context) return false;

    // Set current angle as zero point
    m_context->setEncoderZeroOffset(m_context->encoderAngleDeg() + m_context->encoderTotalAngleDeg());
    return true;
}

QString SimulatedEncoderDevice::lastError() const {
    return QString();
}

} // namespace Simulation
} // namespace Infrastructure
