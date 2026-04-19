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
    double mockVirtualMultiTurnDeg = 0.0;
    double mockAngularVelocityRpm = 0.0;
    bool mockFailReadAngle = false;
    bool mockFailReadVirtualMultiTurn = false;
    bool mockFailReadAngularVelocity = false;
    bool initialized = false;

    explicit MockEncoderDevice(QObject* parent = nullptr)
        : IEncoderDevice(parent) {}

    bool initialize() override { initialized = true; return true; }

    bool readAngle(double& angleDeg) override {
        if (mockFailReadAngle) { m_lastError = "Mock: readAngle failed"; return false; }
        angleDeg = mockAngleDeg;
        return true;
    }

    bool readVirtualMultiTurn(double& totalAngleDeg) override {
        if (mockFailReadVirtualMultiTurn) { m_lastError = "Mock: readVirtualMultiTurn failed"; return false; }
        totalAngleDeg = mockVirtualMultiTurnDeg;
        return true;
    }

    bool readAngularVelocity(double& velocityRpm) override {
        if (mockFailReadAngularVelocity) { m_lastError = "Mock: readAngularVelocity failed"; return false; }
        velocityRpm = mockAngularVelocityRpm;
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
    double mockVoltageV = 0.0;
    double mockPowerW = 0.0;
    int mockMode = 0;
    bool mockFailSetCurrent = false;
    bool mockFailSetOutput = false;
    bool mockFailReadCurrent = false;
    bool outputEnabled = false;
    double lastSetCurrent = 0.0;
    double lastSetVoltage = 0.0;
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

    bool setVoltage(int channel, double voltageV) override {
        lastChannel = channel;
        lastSetVoltage = voltageV;
        mockVoltageV = voltageV;
        return true;
    }

    bool readVoltage(int channel, double& voltageV) override {
        lastChannel = channel;
        voltageV = mockVoltageV;
        return true;
    }

    bool readPower(int channel, double& powerW) override {
        lastChannel = channel;
        powerW = mockPowerW;
        return true;
    }

    bool readMode(int channel, int& mode) override {
        lastChannel = channel;
        mode = mockMode;
        return true;
    }

    bool setBrakeMode(int channel, const QString& mode) override {
        lastChannel = channel;
        // Update mock mode based on mode string
        if (mode == "CV") {
            mockMode = 1;
        } else {
            mockMode = 0;
        }
        return true;
    }

    QString lastError() const override { return m_lastError; }

private:
    QString m_lastError;
};

/**
 * @brief Enhanced Mock Bus Controller for protocol layer testing
 *
 * Provides comprehensive mocking capabilities for Modbus RTU communication testing:
 * - Request/response recording and playback
 * - Configurable delays and timeouts
 * - Error simulation scenarios
 * - CRC validation simulation
 * - Multi-device response mapping
 */
class MockBusController : public Infrastructure::Bus::IBusController {
    Q_OBJECT

public:
    struct RequestRecord {
        QByteArray request;
        QByteArray response;
        qint64 timestamp;
        bool succeeded;
    };

    explicit MockBusController(QObject* parent = nullptr)
        : IBusController(parent)
        , m_isOpen(false)
        , m_mockTimeout(false)
        , m_mockWriteFailure(false)
        , m_responseDelayMs(0)
        , m_requestCount(0)
    {
    }

    // Configuration methods
    void setResponseDelay(int delayMs) { m_responseDelayMs = delayMs; }
    void setMockTimeout(bool enable) { m_mockTimeout = enable; }
    void setMockWriteFailure(bool enable) { m_mockWriteFailure = enable; }
    void setCustomResponse(const QByteArray& response) { m_customResponse = response; }

    // Request recording and analysis
    const QVector<RequestRecord>& getRequestHistory() const { return m_requestHistory; }
    void clearRequestHistory() { m_requestHistory.clear(); m_requestCount = 0; }
    int getRequestCount() const { return m_requestCount; }

    // Response mapping by function code
    using ResponseHandler = std::function<QByteArray(const QByteArray&)>;
    void setResponseHandler(uint8_t functionCode, ResponseHandler handler) {
        m_responseHandlers[functionCode] = handler;
    }
    void clearResponseHandlers() { m_responseHandlers.clear(); }

    // IBusController interface implementation
    bool open(const QString& portName, int baudRate, int timeoutMs,
              const QString& parity = "None", int stopBits = 1) override {
        m_portName = portName;
        m_baudRate = baudRate;
        m_timeoutMs = timeoutMs;
        m_parity = parity;
        m_stopBits = stopBits;
        m_isOpen = true;

        qDebug() << "MockBusController opened:" << portName
                 << "at" << baudRate << "baud";

        return true;
    }

    void close() override {
        m_isOpen = false;
        qDebug() << "MockBusController closed";
    }

    bool isOpen() const override {
        return m_isOpen;
    }

    bool sendRequest(const QByteArray& request, QByteArray& response) override {
        if (!m_isOpen) {
            m_lastError = "MockBusController: port not open";
            return false;
        }

        if (m_mockWriteFailure) {
            m_lastError = "MockBusController: simulated write failure";
            recordRequest(request, QByteArray(), false);
            return false;
        }

        if (m_mockTimeout) {
            m_lastError = QString("MockBusController: simulated timeout after %1 ms").arg(m_timeoutMs);
            recordRequest(request, QByteArray(), false);
            return false;
        }

        // Apply response delay if configured
        if (m_responseDelayMs > 0) {
            QThread::msleep(m_responseDelayMs);
        }

        // Generate response based on request
        response = generateResponse(request);

        recordRequest(request, response, true);
        return true;
    }

    QString lastError() const override {
        return m_lastError;
    }

    // Utility methods for testing
    bool wasFunctionCalled(uint8_t functionCode) const {
        for (const auto& record : m_requestHistory) {
            if (record.request.size() >= 2 &&
                static_cast<uint8_t>(record.request[1]) == functionCode) {
                return true;
            }
        }
        return false;
    }

    int getFunctionCallCount(uint8_t functionCode) const {
        int count = 0;
        for (const auto& record : m_requestHistory) {
            if (record.request.size() >= 2 &&
                static_cast<uint8_t>(record.request[1]) == functionCode) {
                count++;
            }
        }
        return count;
    }

    QByteArray getLastRequest() const {
        if (m_requestHistory.isEmpty()) return QByteArray();
        return m_requestHistory.last().request;
    }

    QByteArray getLastResponse() const {
        if (m_requestHistory.isEmpty()) return QByteArray();
        return m_requestHistory.last().response;
    }

private:
    QByteArray generateResponse(const QByteArray& request) {
        // If custom response is set, use it
        if (!m_customResponse.isEmpty()) {
            return m_customResponse;
        }

        // If request is too short, return empty
        if (request.size() < 2) {
            return QByteArray();
        }

        uint8_t slaveId = static_cast<uint8_t>(request[0]);
        uint8_t functionCode = static_cast<uint8_t>(request[1]);

        // Check if there's a custom handler for this function code
        if (m_responseHandlers.contains(functionCode)) {
            return m_responseHandlers[functionCode](request);
        }

        // Default response generation based on function code
        switch (functionCode) {
            case 0x01: // Read Coils
                return generateReadCoilsResponse(slaveId, request);
            case 0x03: // Read Holding Registers
                return generateReadHoldingRegistersResponse(slaveId, request);
            case 0x04: // Read Input Registers
                return generateReadInputRegistersResponse(slaveId, request);
            case 0x05: // Write Single Coil
                return generateWriteSingleCoilResponse(slaveId, request);
            case 0x06: // Write Single Register
                return generateWriteSingleRegisterResponse(slaveId, request);
            case 0x10: // Write Multiple Registers
                return generateWriteMultipleRegistersResponse(slaveId, request);
            case 0x2B: // Read Device Identification
                return generateReadDeviceIdentificationResponse(slaveId);
            default:
                // Return exception response for unknown function codes
                return generateExceptionResponse(slaveId, functionCode, 0x01); // Illegal function
        }
    }

    QByteArray generateReadCoilsResponse(uint8_t slaveId, const QByteArray& request) {
        if (request.size() < 5) return QByteArray();

        uint16_t startAddr = (static_cast<uint8_t>(request[2]) << 8) | static_cast<uint8_t>(request[3]);
        uint16_t count = (static_cast<uint8_t>(request[4]) << 8) | static_cast<uint8_t>(request[5]);

        uint8_t byteCount = (count + 7) / 8;
        QByteArray response;
        response.append(slaveId);
        response.append(0x01);
        response.append(byteCount);

        // Fill with mock data (all coils ON)
        for (int i = 0; i < byteCount; i++) {
            response.append(0xFF);
        }

        // Add CRC
        uint16_t crc = Infrastructure::Bus::ModbusFrame::calculateCRC16(response);
        response.append(static_cast<char>(crc & 0xFF));
        response.append(static_cast<char>((crc >> 8) & 0xFF));

        return response;
    }

    QByteArray generateReadHoldingRegistersResponse(uint8_t slaveId, const QByteArray& request) {
        if (request.size() < 5) return QByteArray();

        uint16_t count = (static_cast<uint8_t>(request[4]) << 8) | static_cast<uint8_t>(request[5]);

        uint8_t byteCount = count * 2;
        QByteArray response;
        response.append(slaveId);
        response.append(0x03);
        response.append(byteCount);

        // Fill with mock data
        for (uint16_t i = 0; i < count; i++) {
            uint16_t value = 0x0100 + i; // Mock register values
            response.append(static_cast<char>((value >> 8) & 0xFF));
            response.append(static_cast<char>(value & 0xFF));
        }

        // Add CRC
        uint16_t crc = Infrastructure::Bus::ModbusFrame::calculateCRC16(response);
        response.append(static_cast<char>(crc & 0xFF));
        response.append(static_cast<char>((crc >> 8) & 0xFF));

        return response;
    }

    QByteArray generateReadInputRegistersResponse(uint8_t slaveId, const QByteArray& request) {
        if (request.size() < 5) return QByteArray();

        uint16_t count = (static_cast<uint8_t>(request[4]) << 8) | static_cast<uint8_t>(request[5]);

        uint8_t byteCount = count * 2;
        QByteArray response;
        response.append(slaveId);
        response.append(0x04);
        response.append(byteCount);

        // Fill with mock data
        for (uint16_t i = 0; i < count; i++) {
            uint16_t value = 0x0200 + i; // Mock input register values
            response.append(static_cast<char>((value >> 8) & 0xFF));
            response.append(static_cast<char>(value & 0xFF));
        }

        // Add CRC
        uint16_t crc = Infrastructure::Bus::ModbusFrame::calculateCRC16(response);
        response.append(static_cast<char>(crc & 0xFF));
        response.append(static_cast<char>((crc >> 8) & 0xFF));

        return response;
    }

    QByteArray generateWriteSingleCoilResponse(uint8_t slaveId, const QByteArray& request) {
        if (request.size() < 5) return QByteArray();

        // Echo back the request for write confirmation
        QByteArray response = request;
        return response;
    }

    QByteArray generateWriteSingleRegisterResponse(uint8_t slaveId, const QByteArray& request) {
        if (request.size() < 5) return QByteArray();

        // Echo back the request for write confirmation
        QByteArray response = request;
        return response;
    }

    QByteArray generateWriteMultipleRegistersResponse(uint8_t slaveId, const QByteArray& request) {
        if (request.size() < 6) return QByteArray();

        QByteArray response;
        response.append(slaveId);
        response.append(0x10);

        // Echo back address and count
        response.append(request[2]); // Start address hi
        response.append(request[3]); // Start address lo
        response.append(request[4]); // Count hi
        response.append(request[5]); // Count lo

        // Add CRC
        uint16_t crc = Infrastructure::Bus::ModbusFrame::calculateCRC16(response);
        response.append(static_cast<char>(crc & 0xFF));
        response.append(static_cast<char>((crc >> 8) & 0xFF));

        return response;
    }

    QByteArray generateReadDeviceIdentificationResponse(uint8_t slaveId) {
        QByteArray response;
        response.append(slaveId);
        response.append(0x2B);
        response.append(0x0E); // MEI type
        response.append(0x01); // Read Device ID code
        response.append(0x03); // Object count

        // Vendor name
        response.append(static_cast<char>(0x00)); // Object ID
        response.append(static_cast<char>(0x00)); // Object ID
        uint8_t vendorLen = 10;
        response.append(static_cast<char>(vendorLen));
        response.append("MockVendor");

        // Product code
        response.append(static_cast<char>(0x01)); // Object ID
        response.append(static_cast<char>(0x00)); // Object ID
        uint8_t productLen = 12;
        response.append(static_cast<char>(productLen));
        response.append("MockProduct01");

        // Version
        response.append(static_cast<char>(0x02)); // Object ID
        response.append(static_cast<char>(0x00)); // Object ID
        uint8_t versionLen = 8;
        response.append(static_cast<char>(versionLen));
        response.append("v1.0.0");

        // Add CRC
        uint16_t crc = Infrastructure::Bus::ModbusFrame::calculateCRC16(response);
        response.append(static_cast<char>(crc & 0xFF));
        response.append(static_cast<char>((crc >> 8) & 0xFF));

        return response;
    }

    QByteArray generateExceptionResponse(uint8_t slaveId, uint8_t functionCode, uint8_t exceptionCode) {
        QByteArray response;
        response.append(slaveId);
        response.append(functionCode | 0x80); // Set exception bit
        response.append(exceptionCode);

        // Add CRC
        uint16_t crc = Infrastructure::Bus::ModbusFrame::calculateCRC16(response);
        response.append(static_cast<char>(crc & 0xFF));
        response.append(static_cast<char>((crc >> 8) & 0xFF));

        return response;
    }

    void recordRequest(const QByteArray& request, const QByteArray& response, bool succeeded) {
        RequestRecord record;
        record.request = request;
        record.response = response;
        record.timestamp = QDateTime::currentMSecsSinceEpoch();
        record.succeeded = succeeded;

        m_requestHistory.append(record);
        m_requestCount++;
    }

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
