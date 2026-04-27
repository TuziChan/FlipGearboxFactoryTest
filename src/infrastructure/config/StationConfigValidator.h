#ifndef STATIONCONFIGVALIDATOR_H
#define STATIONCONFIGVALIDATOR_H

#include "StationConfig.h"
#include <QString>
#include <QVector>

namespace Infrastructure {
namespace Config {

struct ValidationError {
    QString field;
    QString message;
};

struct ValidationResult {
    bool isValid;
    QVector<ValidationError> errors;

    void addError(const QString& field, const QString& message) {
        isValid = false;
        errors.append({field, message});
    }
};

class StationConfigValidator {
public:
    StationConfigValidator();

    ValidationResult validate(const StationConfig& config);

private:
    bool validateSerialPort(const QString& port);
    bool validateBaudRate(int baudRate);
    bool validateSlaveId(int slaveId);
    bool validateTimeout(int timeout);
    bool validateParity(const QString& parity);
    bool validateStopBits(int stopBits);
    bool validateCommunicationMode(int mode);
    bool validatePollIntervalUs(int intervalUs);
};

} // namespace Config
} // namespace Infrastructure

#endif // STATIONCONFIGVALIDATOR_H
