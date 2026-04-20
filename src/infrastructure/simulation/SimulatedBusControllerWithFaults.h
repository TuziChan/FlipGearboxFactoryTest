#ifndef SIMULATEDBUSCONTROLLERWITHFAULTS_H
#define SIMULATEDBUSCONTROLLERWITHFAULTS_H

#include "src/infrastructure/bus/IBusController.h"
#include <QRandomGenerator>

namespace Infrastructure {
namespace Simulation {

/**
 * @brief Enhanced simulated bus controller with fault injection capabilities
 * 
 * This controller allows testing of various failure scenarios:
 * - Communication timeouts
 * - CRC errors
 * - Partial responses
 * - Data corruption
 * - Intermittent failures
 */
class SimulatedBusControllerWithFaults : public Bus::IBusController {
    Q_OBJECT

public:
    enum class FaultMode {
        None,                  // Normal operation
        AlwaysTimeout,         // All requests timeout
        AlwaysCrcError,        // All responses have CRC errors
        PartialResponse,       // Responses are truncated
        DataCorruption,        // Random data corruption
        IntermittentFailure,   // Random failures (50% rate)
        SlowResponse,          // Delayed responses (near timeout)
        NoResponse,            // Empty responses
        InvalidLength          // Wrong response length
    };

    explicit SimulatedBusControllerWithFaults(QObject* parent = nullptr)
        : IBusController(parent)
        , m_isOpen(false)
        , m_faultMode(FaultMode::None)
        , m_failureCount(0)
        , m_successCount(0)
        , m_timeoutCount(0)
        , m_crcErrorCount(0)
    {
    }

    bool open(const QString& portName,
              int baudRate,
              int timeoutMs,
              const QString& parity = "None",
              int stopBits = 1) override {
        
        if (m_faultMode == FaultMode::AlwaysTimeout) {
            m_lastError = "Simulated timeout during port open";
            return false;
        }
        
        m_portName = portName;
        m_baudRate = baudRate;
        m_timeoutMs = timeoutMs;
        m_parity = parity;
        m_stopBits = stopBits;
        m_isOpen = true;
        return true;
    }

    void close() override {
        m_isOpen = false;
    }

    bool isOpen() const override {
        return m_isOpen;
    }

    bool sendRequest(const QByteArray& request, QByteArray& response) override {
        if (!m_isOpen) {
            m_lastError = "Bus not open";
            m_failureCount++;
            return false;
        }

        // Apply fault injection based on mode
        switch (m_faultMode) {
            case FaultMode::None:
                response.clear();
                m_successCount++;
                return true;

            case FaultMode::AlwaysTimeout:
                m_lastError = "Simulated timeout";
                m_timeoutCount++;
                m_failureCount++;
                return false;

            case FaultMode::AlwaysCrcError:
                // Return corrupted response
                response = QByteArray::fromHex("01030400DEADBEEF");
                m_lastError = "Simulated CRC error";
                m_crcErrorCount++;
                m_failureCount++;
                return false;

            case FaultMode::PartialResponse:
                // Return truncated response
                response = QByteArray::fromHex("0103");
                m_lastError = "Simulated partial response";
                m_failureCount++;
                return false;

            case FaultMode::DataCorruption:
                // Return response with random corruption
                response = QByteArray::fromHex("01030400");
                for (int i = 0; i < 4; i++) {
                    response.append(static_cast<char>(QRandomGenerator::global()->bounded(256)));
                }
                m_lastError = "Simulated data corruption";
                m_failureCount++;
                return false;

            case FaultMode::IntermittentFailure:
                // 50% failure rate
                if (QRandomGenerator::global()->bounded(2) == 0) {
                    m_lastError = "Simulated intermittent failure";
                    m_failureCount++;
                    return false;
                } else {
                    response.clear();
                    m_successCount++;
                    return true;
                }

            case FaultMode::SlowResponse:
                // Simulate slow response (would need actual delay in real implementation)
                response.clear();
                m_successCount++;
                return true;

            case FaultMode::NoResponse:
                response.clear();
                m_lastError = "Simulated no response";
                m_failureCount++;
                return false;

            case FaultMode::InvalidLength:
                // Return response with wrong length
                response = QByteArray::fromHex("010304001122334455667788");
                m_lastError = "Simulated invalid length";
                m_failureCount++;
                return false;
        }

        response.clear();
        m_successCount++;
        return true;
    }

    QString lastError() const override {
        return m_lastError;
    }

    // Fault injection control
    void setFaultMode(FaultMode mode) {
        m_faultMode = mode;
    }

    FaultMode faultMode() const {
        return m_faultMode;
    }

    // Statistics
    int failureCount() const { return m_failureCount; }
    int successCount() const { return m_successCount; }
    int timeoutCount() const { return m_timeoutCount; }
    int crcErrorCount() const { return m_crcErrorCount; }

    void resetStatistics() {
        m_failureCount = 0;
        m_successCount = 0;
        m_timeoutCount = 0;
        m_crcErrorCount = 0;
    }

private:
    bool m_isOpen;
    QString m_portName;
    int m_baudRate;
    int m_timeoutMs;
    QString m_parity;
    int m_stopBits;
    QString m_lastError;
    FaultMode m_faultMode;
    
    // Statistics
    int m_failureCount;
    int m_successCount;
    int m_timeoutCount;
    int m_crcErrorCount;
};

} // namespace Simulation
} // namespace Infrastructure

#endif // SIMULATEDBUSCONTROLLERWITHFAULTS_H
