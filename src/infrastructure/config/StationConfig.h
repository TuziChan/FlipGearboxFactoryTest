#ifndef STATIONCONFIG_H
#define STATIONCONFIG_H

#include <QString>
#include <cstdint>

namespace Infrastructure {
namespace Config {

/**
 * @brief Device communication configuration
 */
struct DeviceConfig {
    QString portName;           // e.g., "COM3"
    uint8_t slaveId = 1;
    int baudRate = 9600;
    int timeout = 500;          // milliseconds
    QString parity = "None";
    int stopBits = 1;
    bool enabled = true;

    // Encoder-specific
    uint16_t encoderResolution = 4096;
};

/**
 * @brief Station/workstation configuration
 */
struct StationConfig {
    QString stationId;
    QString stationName;

    // Device configurations
    DeviceConfig aqmdConfig;
    DeviceConfig dyn200Config;
    DeviceConfig encoderConfig;
    DeviceConfig brakeConfig;

    // Brake channel selection
    int brakeChannel = 1;

    // Default recipe name
    QString defaultRecipe;
};

} // namespace Config
} // namespace Infrastructure

#endif // STATIONCONFIG_H
