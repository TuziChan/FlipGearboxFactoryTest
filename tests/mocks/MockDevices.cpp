#include "MockDevices.h"

namespace Tests {
namespace Mocks {

// MockMotorDevice implementation
MockMotorDevice::MockMotorDevice(QObject* parent)
    : IMotorDriveDevice(parent)
    , mockCurrentA(1.5)
    , mockAi1Level(true)
    , mockFailReadCurrent(false)
    , mockFailReadAI1(false)
    , mockFailSetMotor(false)
    , mockFailBrake(false)
    , lastDirection(Direction::Brake)
    , lastDutyCycle(0.0)
    , initialized(false)
    , magnetDetectionWindowDeg(0.5)
    , enableMagnetDetection(false)
    , linkedEncoderAngle(nullptr)
    , m_lastCheckedAngle(-999.0)
{
    magnetPositionsDeg = {3.0, 49.0, 113.5};
    resetMagnetStates();
}

bool MockMotorDevice::initialize()
{
    initialized = true;
    return true;
}

bool MockMotorDevice::setMotor(Direction direction, double dutyCyclePercent)
{
    if (mockFailSetMotor) {
        m_lastError = "Mock: setMotor failed";
        return false;
    }
    lastDirection = direction;
    lastDutyCycle = dutyCyclePercent;
    return true;
}

bool MockMotorDevice::brake()
{
    if (mockFailBrake) {
        m_lastError = "Mock: brake failed";
        return false;
    }
    lastDirection = Direction::Brake;
    return true;
}

bool MockMotorDevice::coast()
{
    return true;
}

bool MockMotorDevice::readCurrent(double& currentA)
{
    if (mockFailReadCurrent) {
        m_lastError = "Mock: readCurrent failed";
        return false;
    }
    currentA = mockCurrentA;
    return true;
}

bool MockMotorDevice::readAI1Level(bool& level)
{
    if (mockFailReadAI1) {
        m_lastError = "Mock: readAI1Level failed";
        return false;
    }

    if (enableMagnetDetection && linkedEncoderAngle != nullptr) {
        updateMagnetDetection(*linkedEncoderAngle);
    }

    level = mockAi1Level;
    return true;
}

QString MockMotorDevice::lastError() const
{
    return m_lastError;
}

void MockMotorDevice::setMagnetPositions(const QVector<double>& positions)
{
    magnetPositionsDeg = positions;
    resetMagnetStates();
}

void MockMotorDevice::linkEncoderAngle(double* anglePtr)
{
    linkedEncoderAngle = anglePtr;
}

void MockMotorDevice::resetMagnetStates()
{
    magnetStates.clear();
    for (int i = 0; i < magnetPositionsDeg.size(); ++i) {
        magnetStates.append(MagnetState());
    }
}

void MockMotorDevice::setMagnetDetectionEnabled(bool enabled)
{
    enableMagnetDetection = enabled;
    if (enabled) {
        resetMagnetStates();
        mockAi1Level = true;
    }
}

int MockMotorDevice::getMagnetPassCount(int magnetIndex) const
{
    if (magnetIndex >= 0 && magnetIndex < magnetStates.size()) {
        return magnetStates[magnetIndex].passCount;
    }
    return 0;
}

void MockMotorDevice::updateMagnetDetection(double currentAngle)
{
    for (int i = 0; i < magnetPositionsDeg.size(); ++i) {
        double magnetPos = magnetPositionsDeg[i];
        double angleDiff = qAbs(currentAngle - magnetPos);

        if (angleDiff > 180.0) {
            angleDiff = 360.0 - angleDiff;
        }

        if (angleDiff <= magnetDetectionWindowDeg) {
            if (!magnetStates[i].detected) {
                magnetStates[i].detected = true;
                magnetStates[i].detectionAngle = currentAngle;
                magnetStates[i].passCount++;
                mockAi1Level = false;

                qDebug() << QString("MockMotor: Magnet %1 detected at angle %2° (pass #%3)")
                            .arg(i).arg(currentAngle, 0, 'f', 2).arg(magnetStates[i].passCount);
            }
        } else {
            if (magnetStates[i].detected) {
                magnetStates[i].detected = false;
                mockAi1Level = true;
            }
        }
    }

    m_lastCheckedAngle = currentAngle;
}

// MockTorqueDevice implementation
MockTorqueDevice::MockTorqueDevice(QObject* parent)
    : ITorqueSensorDevice(parent)
    , mockTorqueNm(0.5)
    , mockSpeedRpm(1200.0)
    , mockPowerW(60.0)
    , mockFailReadAll(false)
    , initialized(false)
{
}

bool MockTorqueDevice::initialize()
{
    initialized = true;
    return true;
}

bool MockTorqueDevice::readTorque(double& torqueNm)
{
    torqueNm = mockTorqueNm;
    return true;
}

bool MockTorqueDevice::readSpeed(double& speedRpm)
{
    speedRpm = mockSpeedRpm;
    return true;
}

bool MockTorqueDevice::readPower(double& powerW)
{
    powerW = mockPowerW;
    return true;
}

bool MockTorqueDevice::readAll(double& torqueNm, double& speedRpm, double& powerW)
{
    if (mockFailReadAll) {
        m_lastError = "Mock: readAll failed";
        return false;
    }
    torqueNm = mockTorqueNm;
    speedRpm = mockSpeedRpm;
    powerW = mockPowerW;
    return true;
}

QString MockTorqueDevice::lastError() const
{
    return m_lastError;
}

// MockEncoderDevice implementation
MockEncoderDevice::MockEncoderDevice(QObject* parent)
    : IEncoderDevice(parent)
    , mockAngleDeg(0.0)
    , mockVirtualMultiTurnDeg(0.0)
    , mockAngularVelocityRpm(0.0)
    , mockFailReadAngle(false)
    , mockFailReadVirtualMultiTurn(false)
    , mockFailReadAngularVelocity(false)
    , initialized(false)
    , enableAngleSimulation(false)
    , simulationStartAngle(0.0)
    , simulationTargetAngle(0.0)
    , simulationSpeedDegPerTick(0.5)
    , simulationForward(true)
{
}

bool MockEncoderDevice::initialize()
{
    initialized = true;
    return true;
}

bool MockEncoderDevice::readAngle(double& angleDeg)
{
    if (mockFailReadAngle) {
        m_lastError = "Mock: readAngle failed";
        return false;
    }

    if (enableAngleSimulation) {
        updateSimulatedAngle();
    }

    angleDeg = mockAngleDeg;
    return true;
}

bool MockEncoderDevice::readVirtualMultiTurn(double& totalAngleDeg)
{
    if (mockFailReadVirtualMultiTurn) {
        m_lastError = "Mock: readVirtualMultiTurn failed";
        return false;
    }
    totalAngleDeg = mockVirtualMultiTurnDeg;
    return true;
}

bool MockEncoderDevice::readAngularVelocity(double& velocityRpm)
{
    if (mockFailReadAngularVelocity) {
        m_lastError = "Mock: readAngularVelocity failed";
        return false;
    }
    velocityRpm = mockAngularVelocityRpm;
    return true;
}

bool MockEncoderDevice::setZeroPoint()
{
    return true;
}

QString MockEncoderDevice::lastError() const
{
    return m_lastError;
}

void MockEncoderDevice::startAngleSimulation(double startAngle, double targetAngle, double speedDegPerTick, bool forward)
{
    simulationStartAngle = startAngle;
    simulationTargetAngle = targetAngle;
    simulationSpeedDegPerTick = speedDegPerTick;
    simulationForward = forward;
    mockAngleDeg = startAngle;
    enableAngleSimulation = true;
}

void MockEncoderDevice::stopAngleSimulation()
{
    enableAngleSimulation = false;
}

void MockEncoderDevice::setAngle(double angleDeg)
{
    mockAngleDeg = angleDeg;
}

void MockEncoderDevice::updateSimulatedAngle()
{
    if (simulationForward) {
        mockAngleDeg += simulationSpeedDegPerTick;
        if (mockAngleDeg >= simulationTargetAngle) {
            mockAngleDeg = simulationTargetAngle;
            enableAngleSimulation = false;
        }
    } else {
        mockAngleDeg -= simulationSpeedDegPerTick;
        if (mockAngleDeg <= simulationTargetAngle) {
            mockAngleDeg = simulationTargetAngle;
            enableAngleSimulation = false;
        }
    }

    while (mockAngleDeg >= 360.0) mockAngleDeg -= 360.0;
    while (mockAngleDeg < 0.0) mockAngleDeg += 360.0;
}

// MockBrakeDevice implementation
MockBrakeDevice::MockBrakeDevice(QObject* parent)
    : IBrakePowerDevice(parent)
    , mockCurrentA(0.0)
    , mockVoltageV(0.0)
    , mockPowerW(0.0)
    , mockMode(0)
    , mockFailSetCurrent(false)
    , mockFailSetOutput(false)
    , mockFailReadCurrent(false)
    , outputEnabled(false)
    , lastSetCurrent(0.0)
    , lastSetVoltage(0.0)
    , lastChannel(0)
    , initialized(false)
{
}

bool MockBrakeDevice::initialize()
{
    initialized = true;
    return true;
}

bool MockBrakeDevice::setCurrent(int channel, double currentA)
{
    if (mockFailSetCurrent) {
        m_lastError = "Mock: setCurrent failed";
        return false;
    }
    lastChannel = channel;
    lastSetCurrent = currentA;
    mockCurrentA = currentA;
    return true;
}

bool MockBrakeDevice::setOutputEnable(int channel, bool enable)
{
    if (mockFailSetOutput) {
        m_lastError = "Mock: setOutputEnable failed";
        return false;
    }
    lastChannel = channel;
    outputEnabled = enable;
    return true;
}

bool MockBrakeDevice::readCurrent(int channel, double& currentA)
{
    if (mockFailReadCurrent) {
        m_lastError = "Mock: readCurrent failed";
        return false;
    }
    lastChannel = channel;
    currentA = mockCurrentA;
    return true;
}

bool MockBrakeDevice::setVoltage(int channel, double voltageV)
{
    lastChannel = channel;
    lastSetVoltage = voltageV;
    mockVoltageV = voltageV;
    return true;
}

bool MockBrakeDevice::readVoltage(int channel, double& voltageV)
{
    lastChannel = channel;
    voltageV = mockVoltageV;
    return true;
}

bool MockBrakeDevice::readPower(int channel, double& powerW)
{
    lastChannel = channel;
    powerW = mockPowerW;
    return true;
}

bool MockBrakeDevice::readMode(int channel, int& mode)
{
    lastChannel = channel;
    mode = mockMode;
    return true;
}

bool MockBrakeDevice::setBrakeMode(int channel, const QString& mode)
{
    lastChannel = channel;
    if (mode == "CV") {
        mockMode = 1;
    } else {
        mockMode = 0;
    }
    return true;
}

QString MockBrakeDevice::lastError() const
{
    return m_lastError;
}

// MockBusController implementation
MockBusController::MockBusController(QObject* parent)
    : IBusController(parent)
    , m_isOpen(false)
    , m_mockTimeout(false)
    , m_mockWriteFailure(false)
    , m_responseDelayMs(0)
    , m_requestCount(0)
{
}

void MockBusController::setResponseDelay(int delayMs)
{
    m_responseDelayMs = delayMs;
}

void MockBusController::setMockTimeout(bool enable)
{
    m_mockTimeout = enable;
}

void MockBusController::setMockWriteFailure(bool enable)
{
    m_mockWriteFailure = enable;
}

void MockBusController::setCustomResponse(const QByteArray& response)
{
    m_customResponse = response;
}

const QVector<MockBusController::RequestRecord>& MockBusController::getRequestHistory() const
{
    return m_requestHistory;
}

void MockBusController::clearRequestHistory()
{
    m_requestHistory.clear();
    m_requestCount = 0;
}

int MockBusController::getRequestCount() const
{
    return m_requestCount;
}

void MockBusController::setResponseHandler(uint8_t functionCode, ResponseHandler handler)
{
    m_responseHandlers[functionCode] = handler;
}

void MockBusController::clearResponseHandlers()
{
    m_responseHandlers.clear();
}

bool MockBusController::open(const QString& portName, int baudRate, int timeoutMs,
                              const QString& parity, int stopBits)
{
    m_portName = portName;
    m_baudRate = baudRate;
    m_timeoutMs = timeoutMs;
    m_parity = parity;
    m_stopBits = stopBits;
    m_isOpen = true;

    qDebug() << "MockBusController opened:" << portName << "at" << baudRate << "baud";
    return true;
}

void MockBusController::close()
{
    m_isOpen = false;
    qDebug() << "MockBusController closed";
}

bool MockBusController::isOpen() const
{
    return m_isOpen;
}

bool MockBusController::sendRequest(const QByteArray& request, QByteArray& response)
{
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

    if (m_responseDelayMs > 0) {
        QThread::msleep(m_responseDelayMs);
    }

    response = generateResponse(request);
    recordRequest(request, response, true);
    return true;
}

QString MockBusController::lastError() const
{
    return m_lastError;
}

bool MockBusController::wasFunctionCalled(uint8_t functionCode) const
{
    for (const auto& record : m_requestHistory) {
        if (record.request.size() >= 2 &&
            static_cast<uint8_t>(record.request[1]) == functionCode) {
            return true;
        }
    }
    return false;
}

int MockBusController::getFunctionCallCount(uint8_t functionCode) const
{
    int count = 0;
    for (const auto& record : m_requestHistory) {
        if (record.request.size() >= 2 &&
            static_cast<uint8_t>(record.request[1]) == functionCode) {
            count++;
        }
    }
    return count;
}

QByteArray MockBusController::getLastRequest() const
{
    if (m_requestHistory.isEmpty()) return QByteArray();
    return m_requestHistory.last().request;
}

QByteArray MockBusController::getLastResponse() const
{
    if (m_requestHistory.isEmpty()) return QByteArray();
    return m_requestHistory.last().response;
}

QByteArray MockBusController::generateResponse(const QByteArray& request)
{
    if (!m_customResponse.isEmpty()) {
        return m_customResponse;
    }

    if (request.size() < 2) {
        return QByteArray();
    }

    uint8_t slaveId = static_cast<uint8_t>(request[0]);
    uint8_t functionCode = static_cast<uint8_t>(request[1]);

    if (m_responseHandlers.contains(functionCode)) {
        return m_responseHandlers[functionCode](request);
    }

    switch (functionCode) {
        case 0x01:
            return generateReadCoilsResponse(slaveId, request);
        case 0x03:
            return generateReadHoldingRegistersResponse(slaveId, request);
        case 0x04:
            return generateReadInputRegistersResponse(slaveId, request);
        case 0x05:
            return generateWriteSingleCoilResponse(slaveId, request);
        case 0x06:
            return generateWriteSingleRegisterResponse(slaveId, request);
        case 0x10:
            return generateWriteMultipleRegistersResponse(slaveId, request);
        case 0x2B:
            return generateReadDeviceIdentificationResponse(slaveId);
        default:
            return generateExceptionResponse(slaveId, functionCode, 0x01);
    }
}

QByteArray MockBusController::generateReadCoilsResponse(uint8_t slaveId, const QByteArray& request)
{
    if (request.size() < 5) return QByteArray();

    uint16_t count = (static_cast<uint8_t>(request[4]) << 8) | static_cast<uint8_t>(request[5]);
    uint8_t byteCount = (count + 7) / 8;

    QByteArray response;
    response.append(slaveId);
    response.append(0x01);
    response.append(byteCount);

    for (int i = 0; i < byteCount; i++) {
        response.append(0xFF);
    }

    uint16_t crc = Infrastructure::Bus::ModbusFrame::calculateCRC16(response);
    response.append(static_cast<char>(crc & 0xFF));
    response.append(static_cast<char>((crc >> 8) & 0xFF));

    return response;
}

QByteArray MockBusController::generateReadHoldingRegistersResponse(uint8_t slaveId, const QByteArray& request)
{
    if (request.size() < 5) return QByteArray();

    uint16_t count = (static_cast<uint8_t>(request[4]) << 8) | static_cast<uint8_t>(request[5]);
    uint8_t byteCount = count * 2;

    QByteArray response;
    response.append(slaveId);
    response.append(0x03);
    response.append(byteCount);

    for (uint16_t i = 0; i < count; i++) {
        uint16_t value = 0x0100 + i;
        response.append(static_cast<char>((value >> 8) & 0xFF));
        response.append(static_cast<char>(value & 0xFF));
    }

    uint16_t crc = Infrastructure::Bus::ModbusFrame::calculateCRC16(response);
    response.append(static_cast<char>(crc & 0xFF));
    response.append(static_cast<char>((crc >> 8) & 0xFF));

    return response;
}

QByteArray MockBusController::generateReadInputRegistersResponse(uint8_t slaveId, const QByteArray& request)
{
    if (request.size() < 5) return QByteArray();

    uint16_t count = (static_cast<uint8_t>(request[4]) << 8) | static_cast<uint8_t>(request[5]);
    uint8_t byteCount = count * 2;

    QByteArray response;
    response.append(slaveId);
    response.append(0x04);
    response.append(byteCount);

    for (uint16_t i = 0; i < count; i++) {
        uint16_t value = 0x0200 + i;
        response.append(static_cast<char>((value >> 8) & 0xFF));
        response.append(static_cast<char>(value & 0xFF));
    }

    uint16_t crc = Infrastructure::Bus::ModbusFrame::calculateCRC16(response);
    response.append(static_cast<char>(crc & 0xFF));
    response.append(static_cast<char>((crc >> 8) & 0xFF));

    return response;
}

QByteArray MockBusController::generateWriteSingleCoilResponse(uint8_t slaveId, const QByteArray& request)
{
    if (request.size() < 5) return QByteArray();
    return request;
}

QByteArray MockBusController::generateWriteSingleRegisterResponse(uint8_t slaveId, const QByteArray& request)
{
    if (request.size() < 5) return QByteArray();
    return request;
}

QByteArray MockBusController::generateWriteMultipleRegistersResponse(uint8_t slaveId, const QByteArray& request)
{
    if (request.size() < 6) return QByteArray();

    QByteArray response;
    response.append(slaveId);
    response.append(0x10);
    response.append(request[2]);
    response.append(request[3]);
    response.append(request[4]);
    response.append(request[5]);

    uint16_t crc = Infrastructure::Bus::ModbusFrame::calculateCRC16(response);
    response.append(static_cast<char>(crc & 0xFF));
    response.append(static_cast<char>((crc >> 8) & 0xFF));

    return response;
}

QByteArray MockBusController::generateReadDeviceIdentificationResponse(uint8_t slaveId)
{
    QByteArray response;
    response.append(slaveId);
    response.append(0x2B);
    response.append(0x0E);
    response.append(0x01);
    response.append(0x03);

    response.append(static_cast<char>(0x00));
    response.append(static_cast<char>(0x00));
    response.append(static_cast<char>(10));
    response.append("MockVendor");

    response.append(static_cast<char>(0x01));
    response.append(static_cast<char>(0x00));
    response.append(static_cast<char>(12));
    response.append("MockProduct01");

    response.append(static_cast<char>(0x02));
    response.append(static_cast<char>(0x00));
    response.append(static_cast<char>(8));
    response.append("v1.0.0");

    uint16_t crc = Infrastructure::Bus::ModbusFrame::calculateCRC16(response);
    response.append(static_cast<char>(crc & 0xFF));
    response.append(static_cast<char>((crc >> 8) & 0xFF));

    return response;
}

QByteArray MockBusController::generateExceptionResponse(uint8_t slaveId, uint8_t functionCode, uint8_t exceptionCode)
{
    QByteArray response;
    response.append(slaveId);
    response.append(functionCode | 0x80);
    response.append(exceptionCode);

    uint16_t crc = Infrastructure::Bus::ModbusFrame::calculateCRC16(response);
    response.append(static_cast<char>(crc & 0xFF));
    response.append(static_cast<char>((crc >> 8) & 0xFF));

    return response;
}

void MockBusController::recordRequest(const QByteArray& request, const QByteArray& response, bool succeeded)
{
    RequestRecord record;
    record.request = request;
    record.response = response;
    record.timestamp = QDateTime::currentMSecsSinceEpoch();
    record.succeeded = succeeded;

    m_requestHistory.append(record);
    m_requestCount++;
}

} // namespace Mocks
} // namespace Tests
