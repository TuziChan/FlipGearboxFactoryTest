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

int main() {
    std::cout << "sizeof(std::atomic<double>): " << sizeof(std::atomic<double>) << std::endl;
    std::cout << "sizeof(std::atomic<bool>): " << sizeof(std::atomic<bool>) << std::endl;
    std::cout << "sizeof(std::atomic<uint64_t>): " << sizeof(std::atomic<uint64_t>) << std::endl;
    std::cout << "sizeof(std::atomic<int>): " << sizeof(std::atomic<int>) << std::endl;
    std::cout << "sizeof(MotorTelemetry): " << sizeof(MotorTelemetry) << std::endl;
    std::cout << "alignof(MotorTelemetry): " << alignof(MotorTelemetry) << std::endl;
    
    int calculated_padding = HARDWARE_DELETES_FALSE_SHARING - sizeof(std::atomic<double>) -
                             sizeof(std::atomic<bool>) - sizeof(std::atomic<uint64_t>) -
                             sizeof(std::atomic<bool>) - sizeof(std::atomic<int>) - sizeof(std::atomic<int>);
    std::cout << "Calculated padding size: " << calculated_padding << std::endl;
    
    return 0;
}
