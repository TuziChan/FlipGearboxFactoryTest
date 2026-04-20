#ifndef SIMULATIONTESTRUNTIME_H
#define SIMULATIONTESTRUNTIME_H

#include "SimulationContext.h"
#include "SimulationTestHelper.h"
#include "SimulatedMotorDevice.h"
#include "SimulatedTorqueDevice.h"
#include "SimulatedEncoderDevice.h"
#include "SimulatedBrakeDevice.h"
#include "../../domain/GearboxTestEngine.h"
#include <QObject>
#include <memory>

namespace Infrastructure {
namespace Simulation {

/**
 * @brief Complete simulation runtime for testing
 * 
 * Provides a fully configured simulation environment with:
 * - Simulated devices (motor, torque, encoder, brake)
 * - Test engine
 * - Test helper for control and verification
 * - Shared simulation context
 * 
 * This is the main entry point for writing simulation-based tests.
 */
class SimulationTestRuntime : public QObject {
    Q_OBJECT

public:
    explicit SimulationTestRuntime(QObject* parent = nullptr);
    ~SimulationTestRuntime() override;

    // ========== Device Access ==========
    
    /**
     * @brief Get motor device
     */
    SimulatedMotorDevice* motor() const { return m_motor.get(); }
    
    /**
     * @brief Get torque sensor device
     */
    SimulatedTorqueDevice* torque() const { return m_torque.get(); }
    
    /**
     * @brief Get encoder device
     */
    SimulatedEncoderDevice* encoder() const { return m_encoder.get(); }
    
    /**
     * @brief Get brake device
     */
    SimulatedBrakeDevice* brake() const { return m_brake.get(); }
    
    /**
     * @brief Get test engine
     */
    Domain::GearboxTestEngine* testEngine() const { return m_testEngine.get(); }
    
    /**
     * @brief Get test helper
     */
    SimulationTestHelper* helper() const { return m_helper.get(); }
    
    /**
     * @brief Get simulation context
     */
    SimulationContext* context() const { return m_context.get(); }

    // ========== Lifecycle ==========
    
    /**
     * @brief Initialize all devices
     * @return true if successful
     */
    bool initialize();
    
    /**
     * @brief Reset simulation to initial state
     */
    void reset();
    
    /**
     * @brief Check if runtime is initialized
     */
    bool isInitialized() const { return m_initialized; }

    // ========== Configuration ==========
    
    /**
     * @brief Set brake channel (default: 1)
     */
    void setBrakeChannel(int channel);
    
    /**
     * @brief Get brake channel
     */
    int brakeChannel() const { return m_brakeChannel; }

    // ========== Convenience Methods ==========
    
    /**
     * @brief Start test with specified serial number
     * @param serialNumber Serial number for test
     * @return true if test started successfully
     */
    bool startTest(const QString& serialNumber);
    
    /**
     * @brief Get current test state
     */
    Domain::TestRunState currentState() const;
    
    /**
     * @brief Get test results
     */
    Domain::TestResults results() const;

private:
    std::shared_ptr<SimulationContext> m_context;
    std::unique_ptr<SimulationTestHelper> m_helper;
    
    std::unique_ptr<SimulatedMotorDevice> m_motor;
    std::unique_ptr<SimulatedTorqueDevice> m_torque;
    std::unique_ptr<SimulatedEncoderDevice> m_encoder;
    std::unique_ptr<SimulatedBrakeDevice> m_brake;
    
    std::unique_ptr<Domain::GearboxTestEngine> m_testEngine;
    
    int m_brakeChannel;
    bool m_initialized;
};

} // namespace Simulation
} // namespace Infrastructure

#endif // SIMULATIONTESTRUNTIME_H
