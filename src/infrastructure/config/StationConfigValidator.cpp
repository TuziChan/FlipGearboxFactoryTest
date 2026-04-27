#include "StationConfigValidator.h"
#include <QRegularExpression>
#include <QSet>

namespace Infrastructure {
namespace Config {

StationConfigValidator::StationConfigValidator() {
}

ValidationResult StationConfigValidator::validate(const StationConfig& config) {
    ValidationResult result;
    result.isValid = true;

    if (config.stationId.isEmpty()) {
        result.addError("stationId", "Station ID cannot be empty");
    }

    if (config.stationName.isEmpty()) {
        result.addError("stationName", "Station name cannot be empty");
    }

    if (config.brakeChannel < 1 || config.brakeChannel > 4) {
        result.addError("brakeChannel", "Brake channel must be between 1 and 4");
    }

    if (config.aqmdConfig.enabled) {
        if (!validateSerialPort(config.aqmdConfig.portName)) {
            result.addError("aqmd.portName", "Invalid serial port name");
        }
        if (!validateBaudRate(config.aqmdConfig.baudRate)) {
            result.addError("aqmd.baudRate", "Invalid baud rate");
        }
        if (!validateSlaveId(config.aqmdConfig.slaveId)) {
            result.addError("aqmd.slaveId", "Slave ID must be between 1 and 247");
        }
        if (!validateTimeout(config.aqmdConfig.timeout)) {
            result.addError("aqmd.timeout", "Timeout must be between 100 and 5000 ms");
        }
        if (!validateParity(config.aqmdConfig.parity)) {
            result.addError("aqmd.parity", "Invalid parity setting");
        }
        if (!validateStopBits(config.aqmdConfig.stopBits)) {
            result.addError("aqmd.stopBits", "Stop bits must be 1 or 2");
        }
        if (!validateCommunicationMode(config.aqmdConfig.communicationMode)) {
            result.addError("aqmd.communicationMode", "Communication mode must be between 0 and 3");
        }
        if (!validatePollIntervalUs(config.aqmdConfig.pollIntervalUs)) {
            result.addError("aqmd.pollIntervalUs", "Poll interval must be between 100 and 1000000 us");
        }
    }

    if (config.dyn200Config.enabled) {
        if (!validateSerialPort(config.dyn200Config.portName)) {
            result.addError("dyn200.portName", "Invalid serial port name");
        }
        if (!validateBaudRate(config.dyn200Config.baudRate)) {
            result.addError("dyn200.baudRate", "Invalid baud rate");
        }
        if (!validateSlaveId(config.dyn200Config.slaveId)) {
            result.addError("dyn200.slaveId", "Slave ID must be between 1 and 247");
        }
        if (!validateTimeout(config.dyn200Config.timeout)) {
            result.addError("dyn200.timeout", "Timeout must be between 100 and 5000 ms");
        }
        if (!validateParity(config.dyn200Config.parity)) {
            result.addError("dyn200.parity", "Invalid parity setting");
        }
        if (!validateStopBits(config.dyn200Config.stopBits)) {
            result.addError("dyn200.stopBits", "Stop bits must be 1 or 2");
        }
        if (!validateCommunicationMode(config.dyn200Config.communicationMode)) {
            result.addError("dyn200.communicationMode", "Communication mode must be between 0 and 3");
        }
        if (!validatePollIntervalUs(config.dyn200Config.pollIntervalUs)) {
            result.addError("dyn200.pollIntervalUs", "Poll interval must be between 100 and 1000000 us");
        }
    }

    if (config.encoderConfig.enabled) {
        if (!validateSerialPort(config.encoderConfig.portName)) {
            result.addError("encoder.portName", "Invalid serial port name");
        }
        if (!validateBaudRate(config.encoderConfig.baudRate)) {
            result.addError("encoder.baudRate", "Invalid baud rate");
        }
        if (!validateSlaveId(config.encoderConfig.slaveId)) {
            result.addError("encoder.slaveId", "Slave ID must be between 1 and 247");
        }
        if (!validateTimeout(config.encoderConfig.timeout)) {
            result.addError("encoder.timeout", "Timeout must be between 100 and 5000 ms");
        }
        if (config.encoderConfig.encoderResolution == 0) {
            result.addError("encoder.resolution", "Encoder resolution must be greater than 0");
        }
        if (!validateCommunicationMode(config.encoderConfig.communicationMode)) {
            result.addError("encoder.communicationMode", "Communication mode must be between 0 and 4");
        }
        if (!validatePollIntervalUs(config.encoderConfig.pollIntervalUs)) {
            result.addError("encoder.pollIntervalUs", "Poll interval must be between 100 and 1000000 us");
        }
    }

    if (config.brakeConfig.enabled) {
        if (!validateSerialPort(config.brakeConfig.portName)) {
            result.addError("brake.portName", "Invalid serial port name");
        }
        if (!validateBaudRate(config.brakeConfig.baudRate)) {
            result.addError("brake.baudRate", "Invalid baud rate");
        }
        if (!validateSlaveId(config.brakeConfig.slaveId)) {
            result.addError("brake.slaveId", "Slave ID must be between 1 and 247");
        }
        if (!validateTimeout(config.brakeConfig.timeout)) {
            result.addError("brake.timeout", "Timeout must be between 100 and 5000 ms");
        }
        if (!validateCommunicationMode(config.brakeConfig.communicationMode)) {
            result.addError("brake.communicationMode", "Communication mode must be between 0 and 3");
        }
        if (!validatePollIntervalUs(config.brakeConfig.pollIntervalUs)) {
            result.addError("brake.pollIntervalUs", "Poll interval must be between 100 and 1000000 us");
        }
    }

    return result;
}

bool StationConfigValidator::validateSerialPort(const QString& port) {
    QRegularExpression re("^COM([1-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-6])$");
    return re.match(port).hasMatch();
}

bool StationConfigValidator::validateBaudRate(int baudRate) {
    QSet<int> validRates = {9600, 19200, 38400, 57600, 115200};
    return validRates.contains(baudRate);
}

bool StationConfigValidator::validateSlaveId(int slaveId) {
    return slaveId >= 1 && slaveId <= 247;
}

bool StationConfigValidator::validateTimeout(int timeout) {
    return timeout >= 100 && timeout <= 5000;
}

bool StationConfigValidator::validateParity(const QString& parity) {
    QSet<QString> validParities = {"None", "Even", "Odd"};
    return validParities.contains(parity);
}

bool StationConfigValidator::validateStopBits(int stopBits) {
    return stopBits == 1 || stopBits == 2;
}

bool StationConfigValidator::validateCommunicationMode(int mode) {
    return mode >= 0 && mode <= 4;
}

bool StationConfigValidator::validatePollIntervalUs(int intervalUs) {
    return intervalUs >= 100 && intervalUs <= 1000000;
}

} // namespace Config
} // namespace Infrastructure
