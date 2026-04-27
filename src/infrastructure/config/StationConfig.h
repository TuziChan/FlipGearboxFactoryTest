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
    QString portName;
    uint8_t slaveId = 1;
    int baudRate = 9600;
    int timeout = 500;
    QString parity = "None";
    int stopBits = 1;
    bool enabled = true;
    int pollIntervalUs = 5000;
    int communicationMode = 0;  // 0=default mode for each device

    uint16_t encoderResolution = 4096;
};

/**
 * @brief Station/workstation configuration
 */
struct StationConfig {
    QString stationId;
    QString stationName;

    DeviceConfig aqmdConfig;
    DeviceConfig dyn200Config;
    DeviceConfig encoderConfig;
    DeviceConfig brakeConfig;

    int brakeChannel = 1;
    QString defaultRecipe;

    StationConfig() {
        // AQMD3610NS-A2: Even parity, 1 stop bit, 9600 baud (per manual)
        aqmdConfig.slaveId = 1;
        aqmdConfig.parity = "Even";
        aqmdConfig.stopBits = 1;
        aqmdConfig.baudRate = 9600;

        // DYN200: None parity, 2 stop bits, 19200 baud (per manual)
        dyn200Config.slaveId = 2;
        dyn200Config.baudRate = 19200;
        dyn200Config.parity = "None";
        dyn200Config.stopBits = 2;

        // Encoder: None parity, 1 stop bit, 9600 baud (per manual)
        encoderConfig.slaveId = 3;
        encoderConfig.parity = "None";
        encoderConfig.stopBits = 1;
        encoderConfig.baudRate = 9600;

        // Brake (DC power supply): None parity, 1 stop bit, 9600 baud (per manual)
        brakeConfig.slaveId = 4;
        brakeConfig.parity = "None";
        brakeConfig.stopBits = 1;
        brakeConfig.baudRate = 9600;
    }

    /**
     * @brief Create a StationConfig with all device-specific defaults per manual specs.
     *
     * Use this as the single source of truth for default values, ensuring consistency
     * between StationConfig construction and ConfigLoader fallback values.
     */
    static StationConfig defaultConfig() {
        return StationConfig{};
    }
};

} // namespace Config
} // namespace Infrastructure

#endif // STATIONCONFIG_H
