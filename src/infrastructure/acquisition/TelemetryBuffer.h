#ifndef TELEMETRYBUFFER_H
#define TELEMETRYBUFFER_H

#include <atomic>
#include <array>
#include <chrono>
#include <cstdint>

namespace Infrastructure {
namespace Acquisition {

struct alignas(64) MotorTelemetry {
    std::atomic<double> currentA{0.0};
    std::atomic<bool> ai1Level{true};
    std::atomic<uint64_t> timestampNs{0};
    std::atomic<bool> valid{false};
    std::atomic<int> errorCount{0};
    std::atomic<int> successCount{0};
};

struct alignas(64) TorqueTelemetry {
    std::atomic<double> torqueNm{0.0};
    std::atomic<double> speedRpm{0.0};
    std::atomic<double> powerW{0.0};
    std::atomic<uint64_t> timestampNs{0};
    std::atomic<bool> valid{false};
    std::atomic<int> errorCount{0};
    std::atomic<int> successCount{0};
};

struct alignas(64) EncoderTelemetry {
    std::atomic<double> angleDeg{0.0};
    std::atomic<uint64_t> timestampNs{0};
    std::atomic<bool> valid{false};
    std::atomic<int> errorCount{0};
    std::atomic<int> successCount{0};
};

struct alignas(64) BrakeTelemetry {
    std::atomic<double> currentA{0.0};
    std::atomic<uint64_t> timestampNs{0};
    std::atomic<bool> valid{false};
    std::atomic<int> errorCount{0};
    std::atomic<int> successCount{0};
};

struct TelemetryBuffer {
    MotorTelemetry motor;
    TorqueTelemetry torque;
    EncoderTelemetry encoder;
    BrakeTelemetry brake;

    static uint64_t nowNs() {
        auto tp = std::chrono::steady_clock::now();
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
            tp.time_since_epoch());
        return static_cast<uint64_t>(ns.count());
    }
};

struct AcquisitionStats {
    int motorPollHz{0};
    int torquePollHz{0};
    int encoderPollHz{0};
    int brakePollHz{0};
    int motorErrors{0};
    int torqueErrors{0};
    int encoderErrors{0};
    int brakeErrors{0};
};

} // namespace Acquisition
} // namespace Infrastructure

#endif // TELEMETRYBUFFER_H
