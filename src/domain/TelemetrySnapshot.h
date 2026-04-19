#ifndef TELEMETRYSNAPSHOT_H
#define TELEMETRYSNAPSHOT_H

#include <QDateTime>

namespace Domain {

/**
 * @brief Unified telemetry snapshot from all devices
 * 
 * Captured at a single point in time for consistent state machine evaluation.
 */
struct TelemetrySnapshot {
    QDateTime timestamp;
    
    // Motor drive (AQMD)
    double motorCurrentA;
    bool aqmdAi1Level;  // true = high (no magnet), false = low (magnet detected)
    
    // Torque sensor (DYN200)
    double dynSpeedRpm;
    double dynTorqueNm;
    double dynPowerW;
    
    // Encoder
    double encoderAngleDeg;
    
    // Brake power supply
    double brakeCurrentA;
    double brakeVoltageV;
    double brakePowerW;
    
    TelemetrySnapshot()
        : timestamp(QDateTime::currentDateTime())
        , motorCurrentA(0.0)
        , aqmdAi1Level(true)
        , dynSpeedRpm(0.0)
        , dynTorqueNm(0.0)
        , dynPowerW(0.0)
        , encoderAngleDeg(0.0)
        , brakeCurrentA(0.0)
        , brakeVoltageV(0.0)
        , brakePowerW(0.0)
    {}
};

} // namespace Domain

#endif // TELEMETRYSNAPSHOT_H
