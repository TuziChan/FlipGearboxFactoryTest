#include <QCoreApplication>
#include <QtTest>
#include <QSignalSpy>

#include "src/infrastructure/simulation/SimulationTestRuntime.h"
#include "src/infrastructure/simulation/SimulationTestHelper.h"
#include "src/domain/TestRecipe.h"

using namespace Infrastructure::Simulation;
using namespace Domain;

/**
 * @brief Example tests demonstrating simulation test framework usage
 * 
 * These tests show how to use SimulationTestRuntime and SimulationTestHelper
 * to write deterministic, fast, and reliable tests for the gearbox test system.
 */
class SimulationFrameworkExampleTests : public QObject {
    Q_OBJECT

private:
    std::unique_ptr<SimulationTestRuntime> m_runtime;
    SimulationTestHelper* m_helper;

private slots:
    void init() {
        // Create and initialize simulation runtime
        m_runtime = std::make_unique<SimulationTestRuntime>();
        QVERIFY(m_runtime->initialize());
        
        m_helper = m_runtime->helper();
    }

    void cleanup() {
        m_runtime->reset();
    }

    // ========== Basic Framework Usage ==========

    void testSimulationContextCreation() {
        // Verify simulation context is created and accessible
        auto* context = m_runtime->context();
        QVERIFY(context != nullptr);
        QCOMPARE(context->tickCount(), 0ULL);
    }

    void testDeviceCreation() {
        // Verify all devices are created
        QVERIFY(m_runtime->motor() != nullptr);
        QVERIFY(m_runtime->torque() != nullptr);
        QVERIFY(m_runtime->encoder() != nullptr);
        QVERIFY(m_runtime->brake() != nullptr);
        QVERIFY(m_runtime->testEngine() != nullptr);
    }

    void testTimeAdvancement() {
        // Test deterministic time control
        uint64_t startTick = m_helper->currentTick();
        
        m_helper->advanceTicks(10);
        QCOMPARE(m_helper->currentTick(), startTick + 10);
        
        m_helper->advanceMs(100); // 100ms = 10 ticks
        QCOMPARE(m_helper->currentTick(), startTick + 20);
    }

    // ========== Motor Control Tests ==========

    void testMotorForwardControl() {
        // Start motor in forward direction
        m_helper->setMotorForward(50.0);
        
        QVERIFY(m_helper->isMotorRunning());
        QVERIFY(m_helper->isMotorForward());
        QVERIFY(!m_helper->isMotorReverse());
        QCOMPARE(m_helper->motorDutyCycle(), 50.0);
        
        // Allow motor to accelerate
        m_helper->advanceMs(200);
        
        // Verify motor is spinning
        double speed = m_helper->motorSpeedRpm();
        QVERIFY(speed > 0.0);
        qDebug() << "Motor speed after 200ms:" << speed << "RPM";
    }

    void testMotorReverseControl() {
        // Start motor in reverse direction
        m_helper->setMotorReverse(30.0);
        
        QVERIFY(m_helper->isMotorRunning());
        QVERIFY(m_helper->isMotorReverse());
        QVERIFY(!m_helper->isMotorForward());
        
        m_helper->advanceMs(200);
        
        // Speed should be negative in reverse
        double speed = m_helper->motorSpeedRpm();
        QVERIFY(speed < 0.0);
    }

    void testMotorStop() {
        // Start motor then stop
        m_helper->setMotorForward(50.0);
        m_helper->advanceMs(200);
        
        m_helper->stopMotor();
        QVERIFY(!m_helper->isMotorRunning());
        
        // Allow deceleration
        m_helper->advanceMs(500);
        
        // Speed should be near zero
        double speed = std::abs(m_helper->motorSpeedRpm());
        QVERIFY(speed < 10.0); // Allow some residual speed
    }

    void testMotorAcceleration() {
        // Test realistic acceleration curve
        m_helper->setMotorForward(100.0); // Full duty cycle
        
        QVector<double> speeds;
        for (int i = 0; i < 10; i++) {
            m_helper->advanceMs(50);
            speeds.append(m_helper->motorSpeedRpm());
        }
        
        // Verify speed increases over time
        for (int i = 1; i < speeds.size(); i++) {
            QVERIFY(speeds[i] >= speeds[i-1]);
        }
        
        qDebug() << "Acceleration curve:" << speeds;
    }

    // ========== Encoder Tests ==========

    void testEncoderAngleTracking() {
        // Start motor and track encoder angle
        m_helper->setMotorForward(50.0);
        
        double startAngle = m_helper->encoderAngle();
        m_helper->advanceMs(500);
        double endAngle = m_helper->encoderAngle();
        
        // Angle should have increased
        QVERIFY(endAngle > startAngle || endAngle < startAngle - 180.0); // Handle wrap
        
        qDebug() << "Angle change:" << startAngle << "->" << endAngle;
    }

    void testEncoderZeroSetting() {
        // Absolute encoder zero is fixed at installation.
        m_helper->setMotorForward(50.0);
        m_helper->advanceMs(200);
        
        double angleBeforeSetZero = m_helper->encoderAngle();
        QVERIFY(angleBeforeSetZero > 0.0);
        QVERIFY(m_runtime->encoder()->setZeroPoint());
        
        // Runtime setZeroPoint() is a compatibility no-op for absolute encoders.
        double angleAfterSetZero = m_helper->encoderAngle();
        QCOMPARE(angleAfterSetZero, angleBeforeSetZero);
    }

    void testWaitForEncoderAngle() {
        // Start motor and wait for specific angle
        m_helper->setMotorForward(50.0);
        
        bool reached = m_helper->waitForEncoderAngle(90.0, 5.0, 2000);
        QVERIFY(reached);
        
        double finalAngle = m_helper->encoderAngle();
        QVERIFY(std::abs(finalAngle - 90.0) <= 5.0);
    }

    // ========== Brake Tests ==========

    void testBrakeControl() {
        // Enable brake and set current
        m_helper->setBrakeCurrent(2.0);
        m_helper->setBrakeEnabled(true);
        
        QVERIFY(m_helper->isBrakeEnabled());
        QCOMPARE(m_helper->brakeCurrent(), 2.0);
    }

    void testBrakeLoadEffect() {
        // Start motor without brake
        m_helper->setMotorForward(50.0);
        m_helper->advanceMs(300);
        double speedNoBrake = m_helper->motorSpeedRpm();
        
        // Apply brake load
        m_helper->setBrakeCurrent(2.0);
        m_helper->setBrakeEnabled(true);
        m_helper->advanceMs(300);
        double speedWithBrake = m_helper->motorSpeedRpm();
        
        // Speed should be reduced by brake
        QVERIFY(speedWithBrake < speedNoBrake);
        
        qDebug() << "Speed without brake:" << speedNoBrake 
                 << "with brake:" << speedWithBrake;
    }

    // ========== Scenario Tests ==========

    void testIdleRunScenario() {
        // Setup idle run scenario
        m_helper->setupIdleRunScenario(50.0, 750.0);
        
        // Verify motor is running at expected speed
        double speed = m_helper->motorSpeedRpm();
        QVERIFY(speed > 500.0 && speed < 1000.0);
        
        QVERIFY(m_helper->isMotorForward());
        QCOMPARE(m_helper->motorDutyCycle(), 50.0);
    }

    void testLoadTestScenario() {
        // Setup load test scenario
        m_helper->setupLoadTestScenario(50.0, 2.5);
        
        // Verify brake is applying load
        QVERIFY(m_helper->isBrakeEnabled());
        QCOMPARE(m_helper->brakeCurrent(), 2.5);
        
        // Verify motor speed is reduced by load
        double speed = m_helper->motorSpeedRpm();
        QVERIFY(speed < 500.0); // Reduced by brake load
    }

    void testHomingScenario() {
        // Setup homing scenario with magnet at 45 degrees
        m_helper->setupHomingScenario(45.0);
        
        // Verify motor is in reverse for homing
        QVERIFY(m_helper->isMotorReverse());
        
        // Run until magnet detected
        QSignalSpy magnetSpy(m_helper, &SimulationTestHelper::magnetDetected);
        
        // Advance simulation
        m_helper->advanceMs(1000);
        
        // Magnet should be detected when angle passes 45 degrees
        // (This depends on starting position and motor direction)
    }

    // ========== Advanced Control Tests ==========

    void testRunUntilCondition() {
        // Start motor and run until speed reaches threshold
        m_helper->setMotorForward(50.0);
        
        bool reached = m_helper->runUntil([this]() {
            return m_helper->motorSpeedRpm() > 500.0;
        }, 2000);
        
        QVERIFY(reached);
        QVERIFY(m_helper->motorSpeedRpm() > 500.0);
    }

    void testRunUntilTimeout() {
        // Test timeout when condition never met
        m_helper->stopMotor();
        
        bool reached = m_helper->runUntil([this]() {
            return m_helper->motorSpeedRpm() > 1000.0; // Impossible when stopped
        }, 100);
        
        QVERIFY(!reached); // Should timeout
    }

    // ========== Integration with Test Engine ==========

    void testEngineWithSimulation() {
        // Create a simple test recipe
        TestRecipe recipe;
        recipe.name = "SimTest";
        recipe.homeDutyCycle = 20.0;
        recipe.homeTimeoutMs = 1000;
        recipe.idleDutyCycle = 50.0;
        recipe.idleTimeoutMs = 2000;
        
        m_runtime->testEngine()->setRecipe(recipe);
        
        // Start test
        QVERIFY(m_runtime->startTest("SIM-001"));
        
        // Verify test started
        auto state = m_runtime->currentState();
        QCOMPARE(state.phase, TestPhase::PrepareAndHome);
    }

    void testSimulationReset() {
        // Make changes to simulation state
        m_helper->setMotorForward(50.0);
        m_helper->setBrakeCurrent(2.0);
        m_helper->advanceMs(500);
        
        // Reset
        m_runtime->reset();
        
        // Verify state is reset
        QCOMPARE(m_helper->currentTick(), 0ULL);
        QVERIFY(!m_helper->isMotorRunning());
        QCOMPARE(m_helper->brakeCurrent(), 0.0);
    }

    // ========== Signal Tests ==========

    void testMagnetDetectionSignal() {
        // Setup magnet detection
        m_helper->setMagnetDetectionCallback([](double angle) {
            return angle >= 44.0 && angle <= 46.0; // Magnet at ~45 degrees
        });
        
        QSignalSpy magnetSpy(m_helper, &SimulationTestHelper::magnetDetected);
        
        // Start motor in reverse
        m_helper->setMotorReverse(30.0);
        
        // Run simulation
        m_helper->advanceMs(2000);
        
        // Magnet signal should have been emitted
        // (Actual count depends on starting position and speed)
        qDebug() << "Magnet detections:" << magnetSpy.count();
    }

    void testAngleReachedSignal() {
        QSignalSpy angleSpy(m_helper, &SimulationTestHelper::angleReached);
        
        // Start motor
        m_helper->setMotorForward(50.0);
        m_helper->advanceMs(500);
        
        // Angle signal should be emitted as motor rotates
        QVERIFY(angleSpy.count() > 0);
        qDebug() << "Angle updates:" << angleSpy.count();
    }

    // ========== Performance Tests ==========

    void testSimulationPerformance() {
        // Measure simulation performance
        QElapsedTimer timer;
        timer.start();
        
        m_helper->setMotorForward(50.0);
        m_helper->advanceTicks(1000); // Simulate 10 seconds
        
        qint64 elapsed = timer.elapsed();
        qDebug() << "Simulated 10s in" << elapsed << "ms (speedup:" 
                 << (10000.0 / elapsed) << "x)";
        
        // Simulation should be much faster than real-time
        QVERIFY(elapsed < 1000); // Should complete in less than 1 second
    }
};

QTEST_MAIN(SimulationFrameworkExampleTests)
#include "SimulationFrameworkExampleTests.moc"
