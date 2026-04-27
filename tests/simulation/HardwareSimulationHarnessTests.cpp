#include <QTest>
#include <QSignalSpy>
#include <QDebug>
#include "src/infrastructure/simulation/HardwareSimulationHarness.h"
#include "src/infrastructure/simulation/SimulationScenario.h"

using namespace Infrastructure::Simulation;

class HardwareSimulationHarnessTests : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        qDebug() << "HardwareSimulationHarnessTests initializing...";
    }

    void testHarnessLifecycle() {
        HardwareSimulationHarness harness;
        QVERIFY(!harness.isInitialized());

        QVERIFY(harness.initialize());
        QVERIFY(harness.isInitialized());

        harness.shutdown();
        QVERIFY(!harness.isInitialized());
    }

    void testDeviceAccessors() {
        HardwareSimulationHarness harness;
        harness.initialize();

        QVERIFY(harness.motorDevice() != nullptr);
        QVERIFY(harness.torqueDevice() != nullptr);
        QVERIFY(harness.encoderDevice() != nullptr);
        QVERIFY(harness.brakeDevice() != nullptr);

        harness.shutdown();
    }

    void testContextState() {
        HardwareSimulationHarness harness;
        harness.initialize();

        QVERIFY(harness.simulationContext() != nullptr);
        QVERIFY(harness.enhancedContext() != nullptr);

        harness.shutdown();
    }

    void testMotorStateControl() {
        HardwareSimulationHarness harness;
        harness.initialize();

        QSignalSpy spy(&harness, &HardwareSimulationHarness::stateChanged);

        harness.setMotorState(SimulationContext::MotorDirection::Forward, 50.0);
        QCOMPARE(spy.count(), 1);

        harness.setMotorState(SimulationContext::MotorDirection::Reverse, 30.0);
        QCOMPARE(spy.count(), 2);

        harness.setMotorState(SimulationContext::MotorDirection::Stopped, 0.0);
        QCOMPARE(spy.count(), 3);

        harness.shutdown();
    }

    void testBrakeControl() {
        HardwareSimulationHarness harness;
        harness.initialize();

        QSignalSpy spy(&harness, &HardwareSimulationHarness::stateChanged);

        harness.setBrakeOutput(true, 1.5);
        QCOMPARE(spy.count(), 1);

        auto* ctx = harness.simulationContext();
        QVERIFY(ctx->brakeOutputEnabled());
        QCOMPARE(ctx->brakeCurrent(), 1.5);

        harness.shutdown();
    }

    void testEncoderZeroOffset() {
        HardwareSimulationHarness harness;
        harness.initialize();

        // Absolute encoder zero is physically fixed, so forcing the physical
        // angle should be reflected directly in the reported angle.
        if (auto* ec = harness.enhancedContext()) {
            ec->forceEncoderAngle(45.0);
        }
        auto* ctx = harness.simulationContext();
        QCOMPARE(ctx->encoderAngleDeg(), 45.0);

        harness.shutdown();
    }

    void testResetToDefaults() {
        HardwareSimulationHarness harness;
        harness.initialize();

        harness.setMotorState(SimulationContext::MotorDirection::Forward, 80.0);
        harness.setBrakeOutput(true, 2.0);
        harness.injectSensorFault(EnhancedSimulationContext::FaultType::NoiseInjection,
                                   EnhancedSimulationContext::FaultType::None,
                                   EnhancedSimulationContext::FaultType::None);

        harness.resetToDefaults();

        auto* ctx = harness.simulationContext();
        QCOMPARE(ctx->motorDirection(), SimulationContext::MotorDirection::Stopped);
        QCOMPARE(ctx->motorDutyCycle(), 0.0);
        QVERIFY(!ctx->brakeOutputEnabled());
        QCOMPARE(ctx->brakeCurrent(), 0.0);

        auto* ec = harness.enhancedContext();
        QCOMPARE(ec->encoderFault(), EnhancedSimulationContext::FaultType::None);

        harness.shutdown();
    }

    void testBusFaultInjection() {
        HardwareSimulationHarness harness;
        harness.initialize();

        QSignalSpy spy(&harness, &HardwareSimulationHarness::faultInjected);

        harness.injectBusFault(0, SimulatedBusControllerWithFaults::FaultMode::AlwaysTimeout);
        QCOMPARE(spy.count(), 1);

        harness.clearBusFaults();

        harness.shutdown();
    }

    void testSensorFaultInjection() {
        HardwareSimulationHarness harness;
        harness.initialize();

        QSignalSpy spy(&harness, &HardwareSimulationHarness::faultInjected);

        harness.injectSensorFault(EnhancedSimulationContext::FaultType::DataJump,
                                   EnhancedSimulationContext::FaultType::StuckValue,
                                   EnhancedSimulationContext::FaultType::None);
        QCOMPARE(spy.count(), 1);

        auto* ec = harness.enhancedContext();
        QCOMPARE(ec->encoderFault(), EnhancedSimulationContext::FaultType::DataJump);
        QCOMPARE(ec->torqueFault(), EnhancedSimulationContext::FaultType::StuckValue);

        harness.clearSensorFaults();
        QCOMPARE(ec->encoderFault(), EnhancedSimulationContext::FaultType::None);

        harness.shutdown();
    }

    void testMagnetPositions() {
        HardwareSimulationHarness harness;
        harness.initialize();

        QVector<double> positions = {5.0, 50.0, 115.0};
        harness.setMagnetPositions(positions);

        auto retrieved = harness.magnetPositions();
        QCOMPARE(retrieved.size(), 3);
        QCOMPARE(retrieved[0], 5.0);
        QCOMPARE(retrieved[1], 50.0);
        QCOMPARE(retrieved[2], 115.0);

        harness.shutdown();
    }

    void testStatsCollection() {
        HardwareSimulationHarness harness;
        harness.initialize();

        auto stats = harness.stats();
        QVERIFY(stats.motorInitialized);
        QVERIFY(stats.torqueInitialized);
        QVERIFY(stats.encoderInitialized);
        QVERIFY(stats.brakeInitialized);

        harness.shutdown();
    }

    void testScenarioExecution() {
        HardwareSimulationHarness harness;
        harness.initialize();

        SimulationScenario scenario;
        scenario.name = "Basic Motor Spin";
        scenario.description = "Motor forward at 50% for 10 ticks";
        scenario.category = "normal";
        scenario.initialState.motorDirection = SimulationContext::MotorDirection::Forward;
        scenario.initialState.motorDutyCycle = 50.0;
        scenario.passCriteria.maxTicks = 10;

        ScenarioExecutor executor(&harness);
        QSignalSpy tickSpy(&executor, &ScenarioExecutor::tickAdvanced);

        auto result = executor.execute(scenario);
        QVERIFY(result.passed);
        QVERIFY(result.ticksExecuted >= 10);
        QVERIFY(!tickSpy.isEmpty());

        harness.shutdown();
    }

    void testScenarioWithFaults() {
        HardwareSimulationHarness harness;
        harness.initialize();

        SimulationScenario scenario;
        scenario.name = "Fault Injection Test";
        scenario.category = "fault_injection";
        scenario.initialState.motorDirection = SimulationContext::MotorDirection::Forward;
        scenario.initialState.motorDutyCycle = 30.0;

        SimulationScenario::FaultEvent fault;
        fault.tick = 5;
        fault.target = "encoder";
        fault.faultCode = static_cast<int>(EnhancedSimulationContext::FaultType::NoiseInjection);
        scenario.faultSchedule.append(fault);

        scenario.passCriteria.maxTicks = 20;
        scenario.passCriteria.expectSensorFault = true;

        ScenarioExecutor executor(&harness);
        QSignalSpy faultSpy(&executor, &ScenarioExecutor::faultInjected);

        auto result = executor.execute(scenario);
        QVERIFY(result.passed);
        QCOMPARE(faultSpy.count(), 1);

        harness.shutdown();
    }

    void testScenarioJsonRoundTrip() {
        SimulationScenario original;
        original.name = "JSON Test";
        original.description = "Testing serialization";
        original.category = "boundary";
        original.initialState.motorDutyCycle = 75.0;
        original.initialState.brakeCurrentA = 1.2;
        original.passCriteria.maxTicks = 100;

        QJsonObject json = original.toJson();
        auto restored = SimulationScenario::fromJson(json);

        QCOMPARE(restored.name, original.name);
        QCOMPARE(restored.description, original.description);
        QCOMPARE(restored.category, original.category);
        QCOMPARE(restored.initialState.motorDutyCycle, original.initialState.motorDutyCycle);
        QCOMPARE(restored.initialState.brakeCurrentA, original.initialState.brakeCurrentA);
        QCOMPARE(restored.passCriteria.maxTicks, original.passCriteria.maxTicks);
    }

    void cleanupTestCase() {
        qDebug() << "HardwareSimulationHarnessTests completed.";
    }
};

QTEST_MAIN(HardwareSimulationHarnessTests)
#include "HardwareSimulationHarnessTests.moc"
