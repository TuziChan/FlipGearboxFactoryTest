#ifndef MOCKDEVICES_H
#define MOCKDEVICES_H

#include "src/infrastructure/devices/IMotorDriveDevice.h"
#include "src/infrastructure/devices/ITorqueSensorDevice.h"
#include "src/infrastructure/devices/IEncoderDevice.h"
#include "src/infrastructure/devices/IBrakePowerDevice.h"
#include "src/infrastructure/bus/IBusController.h"
#include "src/infrastructure/bus/ModbusFrame.h"
#include <QVector>
#include <QPair>
#include <QMap>
#include <QDebug>
#include <QThread>
#include <QDateTime>
#include <functional>

namespace Tests {
namespace Mocks {

class MockMotorDevice : public Infrastructure::Devices::IMotorDriveDevice {
    Q_OBJECT
public:
    double mockCurrentA;
    bool mockAi1Level;
    bool mockFailReadCurrent;
    bool mockFailReadAI1;
    bool mockFailSetMotor;
    bool mockFailBrake;
    Direction lastDirection;
    double lastDutyCycle;
    bool initialized;

    // Magnet detection configuration
    QVector<double> magnetPositionsDeg;
    double magnetDetectionWindowDeg;
    bool enableMagnetDetection;
    double* linkedEncoderAngle;

    // Magnet detection state tracking
    struct MagnetState {
        bool detected = false;
        double detectionAngle = 0.0;
        int passCount = 0;
    };
    QVector<MagnetState> magnetStates;

    explicit MockMotorDevice(QObject* parent = nullptr);

    bool initialize() override;
    bool setMotor(Direction direction, double dutyCyclePercent) override;
    bool brake() override;
    bool coast() override;
    bool readCurrent(double& currentA) override;
    bool readAI1Level(bool& level) override;
    QString lastError() const override;

    // Magnet detection control methods
    void setMagnetPositions(const QVector<double>& positions);
    void linkEncoderAngle(double* anglePtr);
    void resetMagnetStates();
    void setMagnetDetectionEnabled(bool enabled);
    int getMagnetPassCount(int magnetIndex) const;

private:
    QString m_lastError;
    double m_lastCheckedAngle;
    void updateMagnetDetection(double currentAngle);
};

class MockTorqueDevice : public Infrastructure::Devices::ITorqueSensorDevice {
    Q_OBJECT
public:
    double mockTorqueNm;
    double mockSpeedRpm;
    double mockPowerW;
    bool mockFailReadAll;
    bool initialized;

    explicit MockTorqueDevice(QObject* parent = nullptr);

    bool initialize() override;
    bool readTorque(double& torqueNm) override;
    bool readSpeed(double& speedRpm) override;
    bool readPower(double& powerW) override;
    bool readAll(double& torqueNm, double& speedRpm, double& powerW) override;
    QString lastError() const override;

private:
    QString m_lastError;
};

class MockEncoderDevice : public Infrastructure::Devices::IEncoderDevice {
    Q_OBJECT
public:
    double mockAngleDeg;
    double mockVirtualMultiTurnDeg;
    double mockAngularVelocityRpm;
    bool mockFailReadAngle;
    bool mockFailReadVirtualMultiTurn;
    bool mockFailReadAngularVelocity;
    bool initialized;

    // Angle simulation control
    bool enableAngleSimulation;
    double simulationStartAngle;
    double simulationTargetAngle;
    double simulationSpeedDegPerTick;
    bool simulationForward;

    explicit MockEncoderDevice(QObject* parent = nullptr);

    bool initialize() override;
    bool readAngle(double& angleDeg) override;
    bool readVirtualMultiTurn(double& totalAngleDeg) override;
    bool readAngularVelocity(double& velocityRpm) override;
    bool setZeroPoint() override;
    QString lastError() const override;

    // Simulation control methods
    void startAngleSimulation(double startAngle, double targetAngle, double speedDegPerTick, bool forward);
    void stopAngleSimulation();
    void setAngle(double angleDeg);

private:
    QString m_lastError;
    void updateSimulatedAngle();
};

class MockBrakeDevice : public Infrastructure::Devices::IBrakePowerDevice {
    Q_OBJECT
public:
    double mockCurrentA;
    double mockVoltageV;
    double mockPowerW;
    int mockMode;
    bool mockFailSetCurrent;
    bool mockFailSetOutput;
    bool mockFailReadCurrent;
    bool outputEnabled;
    double lastSetCurrent;
    double lastSetVoltage;
    int lastChannel;
    bool initialized;

    explicit MockBrakeDevice(QObject* parent = nullptr);

    bool initialize() override;
    bool setCurrent(int channel, double currentA) override;
    bool setOutputEnable(int channel, bool enable) override;
    bool readCurrent(int channel, double& currentA) override;
    bool setVoltage(int channel, double voltageV) override;
    bool readVoltage(int channel, double& voltageV) override;
    bool readPower(int channel, double& powerW) override;
    bool readMode(int channel, int& mode) override;
    bool setBrakeMode(int channel, const QString& mode) override;
    QString lastError() const override;

private:
    QString m_lastError;
};

class MockBusController : public Infrastructure::Bus::IBusController {
    Q_OBJECT

public:
    struct RequestRecord {
        QByteArray request;
        QByteArray response;
        qint64 timestamp;
        bool succeeded;
    };

    explicit MockBusController(QObject* parent = nullptr);

    // Configuration methods
    void setResponseDelay(int delayMs);
    void setMockTimeout(bool enable);
    void setMockWriteFailure(bool enable);
    void setCustomResponse(const QByteArray& response);

    // Request recording and analysis
    const QVector<RequestRecord>& getRequestHistory() const;
    void clearRequestHistory();
    int getRequestCount() const;

    // Response mapping by function code
    using ResponseHandler = std::function<QByteArray(const QByteArray&)>;
    void setResponseHandler(uint8_t functionCode, ResponseHandler handler);
    void clearResponseHandlers();

    // IBusController interface implementation
    bool open(const QString& portName, int baudRate, int timeoutMs,
              const QString& parity = "None", int stopBits = 1) override;
    void close() override;
    bool isOpen() const override;
    bool sendRequest(const QByteArray& request, QByteArray& response) override;
    QString lastError() const override;

    // Utility methods for testing
    bool wasFunctionCalled(uint8_t functionCode) const;
    int getFunctionCallCount(uint8_t functionCode) const;
    QByteArray getLastRequest() const;
    QByteArray getLastResponse() const;

private:
    QByteArray generateResponse(const QByteArray& request);
    QByteArray generateReadCoilsResponse(uint8_t slaveId, const QByteArray& request);
    QByteArray generateReadHoldingRegistersResponse(uint8_t slaveId, const QByteArray& request);
    QByteArray generateReadInputRegistersResponse(uint8_t slaveId, const QByteArray& request);
    QByteArray generateWriteSingleCoilResponse(uint8_t slaveId, const QByteArray& request);
    QByteArray generateWriteSingleRegisterResponse(uint8_t slaveId, const QByteArray& request);
    QByteArray generateWriteMultipleRegistersResponse(uint8_t slaveId, const QByteArray& request);
    QByteArray generateReadDeviceIdentificationResponse(uint8_t slaveId);
    QByteArray generateExceptionResponse(uint8_t slaveId, uint8_t functionCode, uint8_t exceptionCode);
    void recordRequest(const QByteArray& request, const QByteArray& response, bool succeeded);

    // Member variables
    bool m_isOpen;
    bool m_mockTimeout;
    bool m_mockWriteFailure;
    int m_responseDelayMs;
    int m_requestCount;
    QString m_lastError;

    // Configuration storage
    QString m_portName;
    int m_baudRate;
    int m_timeoutMs;
    QString m_parity;
    int m_stopBits;

    // Response handling
    QByteArray m_customResponse;
    QVector<RequestRecord> m_requestHistory;
    QMap<uint8_t, ResponseHandler> m_responseHandlers;
};

} // namespace Mocks
} // namespace Tests

#endif // MOCKDEVICES_H
