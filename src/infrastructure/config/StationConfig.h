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
        aqmdConfig.slaveId = 1;
        aqmdConfig.parity = "Even";

        dyn200Config.slaveId = 2;
        dyn200Config.baudRate = 19200;
        dyn200Config.stopBits = 2;

        encoderConfig.slaveId = 3;

        brakeConfig.slaveId = 4;
    }
};

} // namespace Config
} // namespace Infrastructure

#endif // STATIONCONFIG_H
