#ifndef MOCKDEVICES_H
#define MOCKDEVICES_H

#include "src/infrastructure/devices/IMotorDriveDevice.h"
#include "src/infrastructure/devices/ITorqueSensorDevice.h"
#include "src/infrastructure/devices/IEncoderDevice.h"
#include "src/infrastructure/devices/IBrakePowerDevice.h"

namespace Tests {
namespace Mocks {

class MockMotorDevice : public Infrastructure::Devices::IMotorDriveDevice {
    Q_OBJECT
public:
    double mockCurrentA = 1.5;
    bool mockAi1Level = true;
    bool mockFailReadCurrent = false;
    bool mockFailReadAI1 = false;
    bool mockFailSetMotor = false;
    bool mockFailBrake = false;
    Direction lastDirection = Direction::Brake;
    double lastDutyCycle = 0.0;
    bool initialized = false;

    explicit MockMotorDevice(QObject* parent = nullptr)
        : IMotorDriveDevice(parent) {}

    bool initialize() override { initialized = true; return true; }

    bool setMotor(Direction direction, double dutyCyclePercent) override {
        if (mockFailSetMotor) { m_lastError = "Mock: setMotor failed"; return false; }
        lastDirection = direction;
        lastDutyCycle = dutyCyclePercent;
        return true;
    }

    bool brake() override {
        if (mockFailBrake) { m_lastError = "Mock: brake failed"; return false; }
        lastDirection = Direction::Brake;
        return true;
    }

    bool coast() override { return true; }

    bool readCurrent(double& currentA) override {
        if (mockFailReadCurrent) { m_lastError = "Mock: readCurrent failed"; return false; }
        currentA = mockCurrentA;
        return true;
    }

    bool readAI1Level(bool& level) override {
        if (mockFailReadAI1) { m_lastError = "Mock: readAI1Level failed"; return false; }
        level = mockAi1Level;
        return true;
    }

    QString lastError() const override { return m_lastError; }

private:
    QString m_lastError;
};

class MockTorqueDevice : public Infrastructure::Devices::ITorqueSensorDevice {
    Q_OBJECT
public:
    double mockTorqueNm = 0.5;
    double mockSpeedRpm = 1200.0;
    double mockPowerW = 60.0;
    bool mockFailReadAll = false;
    bool initialized = false;

    explicit MockTorqueDevice(QObject* parent = nullptr)
        : ITorqueSensorDevice(parent) {}

    bool initialize() override { initialized = true; return true; }

    bool readTorque(double& torqueNm) override { torqueNm = mockTorqueNm; return true; }
    bool readSpeed(double& speedRpm) override { speedRpm = mockSpeedRpm; return true; }
    bool readPower(double& powerW) override { powerW = mockPowerW; return true; }

    bool readAll(double& torqueNm, double& speedRpm, double& powerW) override {
        if (mockFailReadAll) { m_lastError = "Mock: readAll failed"; return false; }
        torqueNm = mockTorqueNm;
        speedRpm = mockSpeedRpm;
        powerW = mockPowerW;
        return true;
    }

    QString lastError() const override { return m_lastError; }

private:
    QString m_lastError;
};

class MockEncoderDevice : public Infrastructure::Devices::IEncoderDevice {
    Q_OBJECT
public:
    double mockAngleDeg = 0.0;
    bool mockFailReadAngle = false;
    bool initialized = false;

    explicit MockEncoderDevice(QObject* parent = nullptr)
        : IEncoderDevice(parent) {}

    bool initialize() override { initialized = true; return true; }

    bool readAngle(double& angleDeg) override {
        if (mockFailReadAngle) { m_lastError = "Mock: readAngle failed"; return false; }
        angleDeg = mockAngleDeg;
        return true;
    }

    bool setZeroPoint() override { return true; }

    QString lastError() const override { return m_lastError; }

private:
    QString m_lastError;
};

class MockBrakeDevice : public Infrastructure::Devices::IBrakePowerDevice {
    Q_OBJECT
public:
    double mockCurrentA = 0.0;
    bool mockFailSetCurrent = false;
    bool mockFailSetOutput = false;
    bool mockFailReadCurrent = false;
    bool outputEnabled = false;
    double lastSetCurrent = 0.0;
    int lastChannel = 0;
    bool initialized = false;

    explicit MockBrakeDevice(QObject* parent = nullptr)
        : IBrakePowerDevice(parent) {}

    bool initialize() override { initialized = true; return true; }

    bool setCurrent(int channel, double currentA) override {
        if (mockFailSetCurrent) { m_lastError = "Mock: setCurrent failed"; return false; }
        lastChannel = channel;
        lastSetCurrent = currentA;
        mockCurrentA = currentA;
        return true;
    }

    bool setOutputEnable(int channel, bool enable) override {
        if (mockFailSetOutput) { m_lastError = "Mock: setOutputEnable failed"; return false; }
        lastChannel = channel;
        outputEnabled = enable;
        return true;
    }

    bool readCurrent(int channel, double& currentA) override {
        if (mockFailReadCurrent) { m_lastError = "Mock: readCurrent failed"; return false; }
        lastChannel = channel;
        currentA = mockCurrentA;
        return true;
    }

    QString lastError() const override { return m_lastError; }

private:
    QString m_lastError;
};

} // namespace Mocks
} // namespace Tests

#endif // MOCKDEVICES_H
