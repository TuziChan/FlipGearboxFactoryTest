#ifndef TELEMETRYBUFFER_H
#define TELEMETRYBUFFER_H

#include <atomic>
#include <array>
#include <chrono>
#include <cstdint>

namespace Infrastructure {
namespace Acquisition {

// Cache line size for false sharing prevention
#ifndef HARDWARE_DELETES_FALSE_SHARING
#define HARDWARE_DELETES_FALSE_SHARING 64
#endif

struct alignas(HARDWARE_DELETES_FALSE_SHARING) MotorTelemetry {
std::atomic<double> currentA{0.0};
std::atomic<bool> ai1Level{true};
std::atomic<uint64_t> timestampNs{0};
std::atomic<bool> valid{false};
std::atomic<int> errorCount{0};
std::atomic<int> successCount{0};
// Padding to ensure cache line alignment
char _padding[HARDWARE_DELETES_FALSE_SHARING - sizeof(std::atomic<double>) -
sizeof(std::atomic<bool>) - sizeof(std::atomic<uint64_t>) -
sizeof(std::atomic<bool>) - sizeof(std::atomic<int>) - sizeof(std::atomic<int>)];
};

struct alignas(HARDWARE_DELETES_FALSE_SHARING) TorqueTelemetry {
std::atomic<double> torqueNm{0.0};
std::atomic<double> speedRpm{0.0};
std::atomic<double> powerW{0.0};
std::atomic<uint64_t> timestampNs{0};
std::atomic<bool> valid{false};
std::atomic<int> errorCount{0};
std::atomic<int> successCount{0};
// Padding to ensure cache line alignment
char _padding[HARDWARE_DELETES_FALSE_SHARING - sizeof(std::atomic<double>) * 3 -
sizeof(std::atomic<uint64_t>) - sizeof(std::atomic<bool>) -
sizeof(std::atomic<int>) * 2];
};

struct alignas(HARDWARE_DELETES_FALSE_SHARING) EncoderTelemetry {
std::atomic<double> angleDeg{0.0};
std::atomic<double> totalAngleDeg{0.0};
std::atomic<double> velocityRpm{0.0};
std::atomic<uint64_t> timestampNs{0};
std::atomic<bool> valid{false};
std::atomic<int> errorCount{0};
std::atomic<int> successCount{0};
// Padding to ensure cache line alignment
char _padding[HARDWARE_DELETES_FALSE_SHARING - sizeof(std::atomic<double>) * 3 -
sizeof(std::atomic<uint64_t>) - sizeof(std::atomic<bool>) -
sizeof(std::atomic<int>) * 2];
};

struct alignas(HARDWARE_DELETES_FALSE_SHARING) BrakeTelemetry {
std::atomic<double> currentA{0.0};
std::atomic<double> voltageV{0.0};
std::atomic<double> powerW{0.0};
std::atomic<uint64_t> timestampNs{0};
std::atomic<bool> valid{false};
std::atomic<int> errorCount{0};
std::atomic<int> successCount{0};
// Padding to ensure cache line alignment
char _padding[HARDWARE_DELETES_FALSE_SHARING - sizeof(std::atomic<double>) * 3 -
sizeof(std::atomic<uint64_t>) - sizeof(std::atomic<bool>) -
sizeof(std::atomic<int>) * 2];
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
