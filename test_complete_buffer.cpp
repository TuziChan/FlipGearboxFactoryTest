#include <iostream>
#include <atomic>
#include <cstdint>

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
    char _padding[HARDWARE_DELETES_FALSE_SHARING - sizeof(std::atomic<double>) * 3 -
                  sizeof(std::atomic<uint64_t>) - sizeof(std::atomic<bool>) -
                  sizeof(std::atomic<int>) * 2];
};

struct alignas(HARDWARE_DELETES_FALSE_SHARING) EncoderTelemetry {
    std::atomic<double> angleDeg{0.0};
    std::atomic<uint64_t> timestampNs{0};
    std::atomic<bool> valid{false};
    std::atomic<int> errorCount{0};
    std::atomic<int> successCount{0};
    char _padding[HARDWARE_DELETES_FALSE_SHARING - sizeof(std::atomic<double>) -
                  sizeof(std::atomic<uint64_t>) - sizeof(std::atomic<bool>) -
                  sizeof(std::atomic<int>) * 2];
};

struct alignas(HARDWARE_DELETES_FALSE_SHARING) BrakeTelemetry {
    std::atomic<double> currentA{0.0};
    std::atomic<uint64_t> timestampNs{0};
    std::atomic<bool> valid{false};
    std::atomic<int> errorCount{0};
    std::atomic<int> successCount{0};
    char _padding[HARDWARE_DELETES_FALSE_SHARING - sizeof(std::atomic<double>) -
                  sizeof(std::atomic<uint64_t>) - sizeof(std::atomic<bool>) -
                  sizeof(std::atomic<int>) * 2];
};

struct TelemetryBuffer {
    MotorTelemetry motor;
    TorqueTelemetry torque;
    EncoderTelemetry encoder;
    BrakeTelemetry brake;
};

int main() {
    std::cout << "Individual struct sizes:" << std::endl;
    std::cout << "  MotorTelemetry: " << sizeof(MotorTelemetry) << " bytes" << std::endl;
    std::cout << "  TorqueTelemetry: " << sizeof(TorqueTelemetry) << " bytes" << std::endl;
    std::cout << "  EncoderTelemetry: " << sizeof(EncoderTelemetry) << " bytes" << std::endl;
    std::cout << "  BrakeTelemetry: " << sizeof(BrakeTelemetry) << " bytes" << std::endl;
    std::cout << "\nTelemetryBuffer total: " << sizeof(TelemetryBuffer) << " bytes" << std::endl;
    std::cout << "Expected (4 * 128): " << (4 * 128) << " bytes" << std::endl;
    
    std::cout << "\nAlignment verification:" << std::endl;
    std::cout << "  MotorTelemetry alignment: " << alignof(MotorTelemetry) << std::endl;
    std::cout << "  TorqueTelemetry alignment: " << alignof(TorqueTelemetry) << std::endl;
    
    TelemetryBuffer buf;
    std::cout << "\nMember offsets:" << std::endl;
    std::cout << "  motor offset: " << offsetof(TelemetryBuffer, motor) << std::endl;
    std::cout << "  torque offset: " << offsetof(TelemetryBuffer, torque) << std::endl;
    std::cout << "  encoder offset: " << offsetof(TelemetryBuffer, encoder) << std::endl;
    std::cout << "  brake offset: " << offsetof(TelemetryBuffer, brake) << std::endl;
    
    return 0;
}
