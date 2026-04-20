#include <iostream>
#include <atomic>
#include <cstdint>

#ifndef HARDWARE_DELETES_FALSE_SHARING
#define HARDWARE_DELETES_FALSE_SHARING 64
#endif

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

struct alignas(HARDWARE_DELETES_FALSE_SHARING) TelemetryBuffer {
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
    } motor;
    
    TorqueTelemetry torque;
};

int main() {
    std::cout << "sizeof(TorqueTelemetry): " << sizeof(TorqueTelemetry) << std::endl;
    
    int torque_data_size = sizeof(std::atomic<double>) * 3 + sizeof(std::atomic<uint64_t>) + 
                           sizeof(std::atomic<bool>) + sizeof(std::atomic<int>) * 2;
    std::cout << "TorqueTelemetry data size: " << torque_data_size << std::endl;
    std::cout << "TorqueTelemetry padding: " << (64 - torque_data_size) << std::endl;
    
    std::cout << "\nsizeof(TelemetryBuffer): " << sizeof(TelemetryBuffer) << std::endl;
    
    return 0;
}
