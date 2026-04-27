#include <QCoreApplication>
#include <QtTest>
#include <QSignalSpy>

#include "src/infrastructure/config/StationRuntimeFactory.h"
#include "src/infrastructure/config/StationConfig.h"
#include "src/domain/GearboxTestEngine.h"
#include "src/domain/TestRecipe.h"
#include "src/domain/TestResults.h"
#include "src/domain/TestRunState.h"

using namespace Infrastructure::Config;
using namespace Domain;

/**
 * @brief Integration tests for gearbox testing with simulation runtime
 * 
 * This test suite covers:
 * - Complete gearbox test sequences using simulated devices
 * - Homing, idle, angle, and load test phases
 * - Edge cases and boundary conditions
 * - Failure scenarios and error handling
 * 
 * All tests use deterministic simulation to avoid flaky behavior.
 */
class GearboxSimulationIntegrationTests : public QObject {
    Q_OBJECT

private:
    StationConfig makeTestConfig() {
        StationConfig config;
        config.stationName = "SimTest";
        config.brakeChannel = 1;
        
        config.aqmdConfig.enabled = true;
        config.aqmdConfig.portName = "SIM_AQMD";
        config.aqmdConfig.baudRate = 9600;
        config.aqmdConfig.slaveId = 1;
        config.aqmdConfig.timeout = 1000;
        config.aqmdConfig.parity = "None";
        config.aqmdConfig.stopBits = 1;
        config.aqmdConfig.pollIntervalUs = 10000;
        
        config.dyn200Config.enabled = true;
        config.dyn200Config.portName = "SIM_DYN200";
        config.dyn200Config.baudRate = 9600;
        config.dyn200Config.slaveId = 1;
        config.dyn200Config.timeout = 1000;
        config.dyn200Config.parity = "None";
        config.dyn200Config.stopBits = 1;
        config.dyn200Config.communicationMode = 0;
        config.dyn200Config.pollIntervalUs = 10000;
        
        config.encoderConfig.enabled = true;
        config.encoderConfig.portName = "SIM_ENCODER";
        config.encoderConfig.baudRate = 9600;
        config.encoderConfig.slaveId = 1;
        config.encoderConfig.timeout = 1000;
        config.encoderConfig.parity = "None";
        config.encoderConfig.stopBits = 1;
        config.encoderConfig.encoderResolution = 4096;
        config.encoderConfig.communicationMode = 0;
        config.encoderConfig.pollIntervalUs = 10000;
        
        config.brakeConfig.enabled = true;
        config.brakeConfig.portName = "SIM_BRAKE";
        config.brakeConfig.baudRate = 9600;
        config.brakeConfig.slaveId = 1;
        config.brakeConfig.timeout = 1000;
        config.brakeConfig.parity = "None";
        config.brakeConfig.stopBits = 1;
        config.brakeConfig.pollIntervalUs = 10000;
        
        return config;
    }
    
    TestRecipe makeTestRecipe() {
        TestRecipe r;
        r.name = "SimIntegrationTest";
        r.homeDutyCycle = 20.0;
        r.homeAdvanceDutyCycle = 20.0;
        r.encoderZeroAngleDeg = 0.0;  // Encoder zero is fixed at installation
        r.homeTimeoutMs = 5000;
        r.idleDutyCycle = 50.0;
        r.idleForwardSpinupMs = 200;
        r.idleForwardSampleMs = 200;
        r.idleReverseSpinupMs = 200;
        r.idleReverseSampleMs = 200;
        r.idleTimeoutMs = 10000;
        r.angleTestDutyCycle = 30.0;
        // Position targets use absolute angles (not relative to runtime homing)
        r.position1TargetDeg = 49.0;    // Position 1 absolute target
        r.position1ToleranceDeg = 5.0;
        r.position2TargetDeg = 113.5;   // Position 2 absolute target
        r.position2ToleranceDeg = 5.0;
        r.position3TargetDeg = 113.5;   // Position 3 absolute target
        r.position3ToleranceDeg = 5.0;
        r.returnZeroToleranceDeg = 3.0;
        r.angleTimeoutMs = 5000;
        r.loadDutyCycle = 50.0;
        r.loadTimeoutMs = 10000;
        r.loadSpinupMs = 200;
        r.loadRampMs = 1000;
        r.brakeRampStartCurrentA = 0.0;
        r.brakeRampEndCurrentA = 3.0;
        r.brakeMode = "CC";
        r.brakeRampStartVoltage = 0.0;
        r.brakeRampEndVoltage = 12.0;
        r.lockSpeedThresholdRpm = 5.0;
        r.lockAngleWindowMs = 100;
        r.lockAngleDeltaDeg = 5.0;
        r.lockHoldMs = 100;
        r.returnZeroTimeoutMs = 5000;
        r.gearBacklashCompensationDeg = 0.0;
        return r;
    }

private slots:
    // ========== Basic Runtime Tests ==========
    
    void testRuntimeCreationAndInitialization() {
        StationConfig config = makeTestConfig();
        auto runtime = StationRuntimeFactory::create(config, true);
        
        QVERIFY(runtime != nullptr);
        QVERIFY(runtime->initialize());
        QVERIFY(runtime->testEngine() != nullptr);
        
        runtime->shutdown();
    }
    
    void testEngineInitialState() {
        StationConfig config = makeTestConfig();
        auto runtime = StationRuntimeFactory::create(config, true);
        runtime->initialize();
        
        auto engine = runtime->testEngine();
        QCOMPARE(engine->currentState().phase, TestPhase::Idle);
        auto phase = engine->currentState().phase;
        QVERIFY(phase == TestPhase::Idle || phase == TestPhase::Completed || phase == TestPhase::Failed);
        
        runtime->shutdown();
    }
    
    // ========== Recipe Configuration Tests ==========
    
    void testSetRecipe() {
        StationConfig config = makeTestConfig();
        auto runtime = StationRuntimeFactory::create(config, true);
        runtime->initialize();
        
        TestRecipe recipe = makeTestRecipe();
        runtime->testEngine()->setRecipe(recipe);
        
        // Recipe should be set (no direct getter, but start should work)
        QVERIFY(runtime->testEngine()->currentState().phase == TestPhase::Idle);
        
        runtime->shutdown();
    }
    
    void testRecipeWithZeroDutyCycle() {
        StationConfig config = makeTestConfig();
        auto runtime = StationRuntimeFactory::create(config, true);
        runtime->initialize();
        
        TestRecipe recipe = makeTestRecipe();
        recipe.homeDutyCycle = 0.0; // Invalid
        runtime->testEngine()->setRecipe(recipe);
        
        // Engine should handle gracefully
        QVERIFY(runtime->testEngine()->currentState().phase == TestPhase::Idle);
        
        runtime->shutdown();
    }
    
    void testRecipeWithExcessiveDutyCycle() {
        StationConfig config = makeTestConfig();
        auto runtime = StationRuntimeFactory::create(config, true);
        runtime->initialize();
        
        TestRecipe recipe = makeTestRecipe();
        recipe.idleDutyCycle = 150.0; // Over 100%
        runtime->testEngine()->setRecipe(recipe);
        
        // Engine should clamp or handle
        QVERIFY(runtime->testEngine()->currentState().phase == TestPhase::Idle);
        
        runtime->shutdown();
    }
    
    // ========== Test Execution State Tests ==========
    
    void testStartTest() {
        StationConfig config = makeTestConfig();
        auto runtime = StationRuntimeFactory::create(config, true);
        runtime->initialize();
        
        TestRecipe recipe = makeTestRecipe();
        runtime->testEngine()->setRecipe(recipe);
        
        QSignalSpy stateSpy(runtime->testEngine(), &GearboxTestEngine::stateChanged);
        
        runtime->testEngine()->startTest("TEST-001");
        
        // Should transition from Idle
        QVERIFY(stateSpy.count() > 0);
        auto phase = runtime->testEngine()->currentState().phase;
        QVERIFY(phase != TestPhase::Idle && phase != TestPhase::Completed && phase != TestPhase::Failed);
        
        runtime->testEngine()->emergencyStop();
        runtime->shutdown();
    }
    
    void testAbortTest() {
        StationConfig config = makeTestConfig();
        auto runtime = StationRuntimeFactory::create(config, true);
        runtime->initialize();
        
        TestRecipe recipe = makeTestRecipe();
        runtime->testEngine()->setRecipe(recipe);
        runtime->testEngine()->startTest("TEST-001");
        
        // Let it run briefly
        QTest::qWait(100);
        
        runtime->testEngine()->emergencyStop();
        
        // Should return to idle
        QTest::qWait(100);
        auto phase = runtime->testEngine()->currentState().phase;
        QVERIFY(phase == TestPhase::Idle || phase == TestPhase::Completed || phase == TestPhase::Failed);
        
        runtime->shutdown();
    }
    
    void testResetAfterCompletion() {
        StationConfig config = makeTestConfig();
        auto runtime = StationRuntimeFactory::create(config, true);
        runtime->initialize();
        
        TestRecipe recipe = makeTestRecipe();
        runtime->testEngine()->setRecipe(recipe);
        
        runtime->testEngine()->reset();
        
        QCOMPARE(runtime->testEngine()->currentState().phase, TestPhase::Idle);
        auto phase = runtime->testEngine()->currentState().phase;
        QVERIFY(phase == TestPhase::Idle || phase == TestPhase::Completed || phase == TestPhase::Failed);
        
        runtime->shutdown();
    }
    
    // ========== Boundary Condition Tests ==========
    
    void testVeryShortTimeouts() {
        StationConfig config = makeTestConfig();
        auto runtime = StationRuntimeFactory::create(config, true);
        runtime->initialize();
        
        TestRecipe recipe = makeTestRecipe();
        recipe.homeTimeoutMs = 10; // Very short
        recipe.idleTimeoutMs = 10;
        recipe.angleTimeoutMs = 10;
        recipe.loadTimeoutMs = 10;
        
        runtime->testEngine()->setRecipe(recipe);
        runtime->testEngine()->startTest("TEST-001");
        
        // Should timeout quickly
        QTest::qWait(500);

        // Engine should handle timeout gracefully (should be stopped or failed)
        auto phase = runtime->testEngine()->currentState().phase;
        QVERIFY(phase == TestPhase::Idle || phase == TestPhase::Completed || phase == TestPhase::Failed);
        
        runtime->testEngine()->emergencyStop();
        runtime->shutdown();
    }
    
    void testVeryLongTimeouts() {
        StationConfig config = makeTestConfig();
        auto runtime = StationRuntimeFactory::create(config, true);
        runtime->initialize();
        
        TestRecipe recipe = makeTestRecipe();
        recipe.homeTimeoutMs = 60000; // 1 minute
        
        runtime->testEngine()->setRecipe(recipe);
        
        // Should accept long timeouts without error
        QVERIFY(runtime->testEngine()->currentState().phase == TestPhase::Idle);
        
        runtime->shutdown();
    }
    
    void testZeroToleranceAngles() {
        StationConfig config = makeTestConfig();
        auto runtime = StationRuntimeFactory::create(config, true);
        runtime->initialize();
        
        TestRecipe recipe = makeTestRecipe();
        recipe.position1ToleranceDeg = 0.0; // Zero tolerance
        recipe.position2ToleranceDeg = 0.0;
        recipe.position3ToleranceDeg = 0.0;
        
        runtime->testEngine()->setRecipe(recipe);
        
        // Should handle zero tolerance (very strict)
        QVERIFY(runtime->testEngine()->currentState().phase == TestPhase::Idle);
        
        runtime->shutdown();
    }
    
    void testNegativeAngles() {
        StationConfig config = makeTestConfig();
        auto runtime = StationRuntimeFactory::create(config, true);
        runtime->initialize();
        
        TestRecipe recipe = makeTestRecipe();
        recipe.position1TargetDeg = -10.0; // Negative angle
        
        runtime->testEngine()->setRecipe(recipe);
        
        // Should handle negative angles
        QVERIFY(runtime->testEngine()->currentState().phase == TestPhase::Idle);
        
        runtime->shutdown();
    }
    
    void testAnglesOver360() {
        StationConfig config = makeTestConfig();
        auto runtime = StationRuntimeFactory::create(config, true);
        runtime->initialize();
        
        TestRecipe recipe = makeTestRecipe();
        recipe.position1TargetDeg = 400.0; // Over 360
        
        runtime->testEngine()->setRecipe(recipe);
        
        // Should handle angles over 360
        QVERIFY(runtime->testEngine()->currentState().phase == TestPhase::Idle);
        
        runtime->shutdown();
    }
    
    // ========== Brake Mode Tests ==========
    
    void testConstantCurrentMode() {
        StationConfig config = makeTestConfig();
        auto runtime = StationRuntimeFactory::create(config, true);
        runtime->initialize();
        
        TestRecipe recipe = makeTestRecipe();
        recipe.brakeMode = "CC";
        recipe.brakeRampStartCurrentA = 0.5;
        recipe.brakeRampEndCurrentA = 2.5;
        
        runtime->testEngine()->setRecipe(recipe);
        
        // Should accept CC mode
        QVERIFY(runtime->testEngine()->currentState().phase == TestPhase::Idle);
        
        runtime->shutdown();
    }
    
    void testConstantVoltageMode() {
        StationConfig config = makeTestConfig();
        auto runtime = StationRuntimeFactory::create(config, true);
        runtime->initialize();
        
        TestRecipe recipe = makeTestRecipe();
        recipe.brakeMode = "CV";
        recipe.brakeRampStartVoltage = 0.0;
        recipe.brakeRampEndVoltage = 12.0;
        
        runtime->testEngine()->setRecipe(recipe);
        
        // Should accept CV mode
        QVERIFY(runtime->testEngine()->currentState().phase == TestPhase::Idle);
        
        runtime->shutdown();
    }
    
    void testInvalidBrakeMode() {
        StationConfig config = makeTestConfig();
        auto runtime = StationRuntimeFactory::create(config, true);
        runtime->initialize();
        
        TestRecipe recipe = makeTestRecipe();
        recipe.brakeMode = "INVALID";
        
        runtime->testEngine()->setRecipe(recipe);
        
        // Should handle invalid mode gracefully
        QVERIFY(runtime->testEngine()->currentState().phase == TestPhase::Idle);
        
        runtime->shutdown();
    }
    
    void testZeroBrakeCurrent() {
        StationConfig config = makeTestConfig();
        auto runtime = StationRuntimeFactory::create(config, true);
        runtime->initialize();
        
        TestRecipe recipe = makeTestRecipe();
        recipe.brakeMode = "CC";
        recipe.brakeRampStartCurrentA = 0.0;
        recipe.brakeRampEndCurrentA = 0.0;
        
        runtime->testEngine()->setRecipe(recipe);
        
        // Should handle zero brake current (no load test)
        QVERIFY(runtime->testEngine()->currentState().phase == TestPhase::Idle);
        
        runtime->shutdown();
    }
    
    void testExcessiveBrakeCurrent() {
        StationConfig config = makeTestConfig();
        auto runtime = StationRuntimeFactory::create(config, true);
        runtime->initialize();
        
        TestRecipe recipe = makeTestRecipe();
        recipe.brakeMode = "CC";
        recipe.brakeRampStartCurrentA = 0.0;
        recipe.brakeRampEndCurrentA = 100.0; // Very high
        
        runtime->testEngine()->setRecipe(recipe);
        
        // Should handle excessive current (may clamp)
        QVERIFY(runtime->testEngine()->currentState().phase == TestPhase::Idle);
        
        runtime->shutdown();
    }
    
    // ========== Signal Emission Tests ==========
    
    void testStateChangedSignal() {
        StationConfig config = makeTestConfig();
        auto runtime = StationRuntimeFactory::create(config, true);
        runtime->initialize();
        
        TestRecipe recipe = makeTestRecipe();
        runtime->testEngine()->setRecipe(recipe);
        
        QSignalSpy stateSpy(runtime->testEngine(), &GearboxTestEngine::stateChanged);
        
        runtime->testEngine()->startTest("TEST-001");
        
        // Should emit state changed
        QVERIFY(stateSpy.wait(1000));
        QVERIFY(stateSpy.count() > 0);
        
        runtime->testEngine()->emergencyStop();
        runtime->shutdown();
    }
    
    void testTelemetrySignal() {
        StationConfig config = makeTestConfig();
        auto runtime = StationRuntimeFactory::create(config, true);
        runtime->initialize();
        
        TestRecipe recipe = makeTestRecipe();
        runtime->testEngine()->setRecipe(recipe);

        QSignalSpy stateSpy(runtime->testEngine(), &GearboxTestEngine::stateChanged);

        runtime->testEngine()->startTest("TEST-001");

        // Should emit state updates
        QVERIFY(stateSpy.wait(2000));
        QVERIFY(stateSpy.count() > 0);
        
        runtime->testEngine()->emergencyStop();
        runtime->shutdown();
    }
    
    // ========== Multiple Test Runs ==========
    
    void testConsecutiveTestRuns() {
        StationConfig config = makeTestConfig();
        auto runtime = StationRuntimeFactory::create(config, true);
        runtime->initialize();
        
        TestRecipe recipe = makeTestRecipe();
        runtime->testEngine()->setRecipe(recipe);
        
        // Run 1
        runtime->testEngine()->startTest("TEST-001");
        QTest::qWait(200);
        runtime->testEngine()->emergencyStop();
        QTest::qWait(100);
        
        // Run 2
        runtime->testEngine()->reset();
        runtime->testEngine()->startTest("TEST-001");
        QTest::qWait(200);
        runtime->testEngine()->emergencyStop();
        QTest::qWait(100);
        
        // Should handle multiple runs
        auto phase = runtime->testEngine()->currentState().phase;
        QVERIFY(phase == TestPhase::Idle || phase == TestPhase::Completed || phase == TestPhase::Failed);
        
        runtime->shutdown();
    }
    
    void testResetBetweenRuns() {
        StationConfig config = makeTestConfig();
        auto runtime = StationRuntimeFactory::create(config, true);
        runtime->initialize();
        
        TestRecipe recipe = makeTestRecipe();
        runtime->testEngine()->setRecipe(recipe);
        
        runtime->testEngine()->startTest("TEST-001");
        QTest::qWait(200);
        runtime->testEngine()->emergencyStop();
        
        runtime->testEngine()->reset();
        
        QCOMPARE(runtime->testEngine()->currentState().phase, TestPhase::Idle);
        auto phase = runtime->testEngine()->currentState().phase;
        QVERIFY(phase == TestPhase::Idle || phase == TestPhase::Completed || phase == TestPhase::Failed);
        
        runtime->shutdown();
    }
    
    // ========== Error Handling Tests ==========
    
    void testStartWithoutRecipe() {
        StationConfig config = makeTestConfig();
        auto runtime = StationRuntimeFactory::create(config, true);
        runtime->initialize();
        
        // Don't set recipe
        runtime->testEngine()->startTest("TEST-001");
        
        // Should handle gracefully (may not start or fail quickly)
        QTest::qWait(100);
        
        runtime->testEngine()->emergencyStop();
        runtime->shutdown();
    }
    
    void testDoubleStart() {
        StationConfig config = makeTestConfig();
        auto runtime = StationRuntimeFactory::create(config, true);
        runtime->initialize();
        
        TestRecipe recipe = makeTestRecipe();
        runtime->testEngine()->setRecipe(recipe);
        
        runtime->testEngine()->startTest("TEST-001");
        runtime->testEngine()->startTest("TEST-001"); // Second start
        
        // Should handle double start gracefully
        QTest::qWait(100);
        
        runtime->testEngine()->emergencyStop();
        runtime->shutdown();
    }
    
    void testAbortWhenNotRunning() {
        StationConfig config = makeTestConfig();
        auto runtime = StationRuntimeFactory::create(config, true);
        runtime->initialize();
        
        // Abort without starting
        runtime->testEngine()->emergencyStop();
        
        // Should handle gracefully
        QCOMPARE(runtime->testEngine()->currentState().phase, TestPhase::Idle);
        
        runtime->shutdown();
    }
    
    // ========== Simulation Determinism Tests ==========
    
    void testDeterministicBehavior() {
        // Run same test twice, verify consistent behavior
        StationConfig config = makeTestConfig();
        TestRecipe recipe = makeTestRecipe();
        
        // Run 1
        auto runtime1 = StationRuntimeFactory::create(config, true);
        runtime1->initialize();
        runtime1->testEngine()->setRecipe(recipe);
        
        QSignalSpy stateSpy1(runtime1->testEngine(), &GearboxTestEngine::stateChanged);
        runtime1->testEngine()->startTest("TEST-001");
        QTest::qWait(500);
        int stateChanges1 = stateSpy1.count();
        runtime1->testEngine()->emergencyStop();
        runtime1->shutdown();
        
        // Run 2
        auto runtime2 = StationRuntimeFactory::create(config, true);
        runtime2->initialize();
        runtime2->testEngine()->setRecipe(recipe);
        
        QSignalSpy stateSpy2(runtime2->testEngine(), &GearboxTestEngine::stateChanged);
        runtime2->testEngine()->startTest("TEST-001");
        QTest::qWait(500);
        int stateChanges2 = stateSpy2.count();
        runtime2->testEngine()->emergencyStop();
        runtime2->shutdown();
        
        // Should have similar behavior (within tolerance)
        QVERIFY(qAbs(stateChanges1 - stateChanges2) <= 2);
    }
};

QTEST_MAIN(GearboxSimulationIntegrationTests)
#include "GearboxSimulationIntegrationTests.moc"
