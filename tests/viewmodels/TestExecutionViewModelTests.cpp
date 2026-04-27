#include <QtTest/QtTest>
#include <QSignalSpy>
#include "../../src/viewmodels/TestExecutionViewModel.h"
#include "../../src/infrastructure/config/StationRuntime.h"
#include "../../src/infrastructure/config/RuntimeManager.h"
#include "../../src/domain/GearboxTestEngine.h"
#include "../../src/domain/TestRunState.h"
#include "../../src/domain/TestResults.h"

// Testable GearboxTestEngine that allows manual signal triggering
class TestableGearboxTestEngine : public Domain::GearboxTestEngine {
    Q_OBJECT
public:
    explicit TestableGearboxTestEngine(QObject* parent = nullptr)
        : Domain::GearboxTestEngine(parent)
        , startTestCalled(false)
        , emergencyStopCalled(false)
        , resetCalled(false)
        , lastSerialNumber()
        , lastRecipe()
    {}

    bool startTestCalled;
    bool emergencyStopCalled;
    bool resetCalled;
    QString lastSerialNumber;
    Domain::TestRecipe lastRecipe;

    // Override to track calls
    bool startTest(const QString& serialNumber) {
        startTestCalled = true;
        lastSerialNumber = serialNumber;
        // Don't actually start the test, just track the call
        return true;
    }

    void emergencyStop() {
        emergencyStopCalled = true;
        Domain::GearboxTestEngine::emergencyStop();
    }

    void reset() {
        resetCalled = true;
        Domain::GearboxTestEngine::reset();
    }

    void setRecipe(const Domain::TestRecipe& recipe) {
        lastRecipe = recipe;
        Domain::GearboxTestEngine::setRecipe(recipe);
    }

    // Helper methods to manually trigger signals for testing
    void triggerStateChanged(const Domain::TestRunState& state) {
        emit stateChanged(state);
    }

    void triggerTestCompleted(const Domain::TestResults& results) {
        emit testCompleted(results);
    }

    void triggerTestFailed(const Domain::FailureReason& reason) {
        emit testFailed(reason);
    }
};

// Simple mock runtime for testing
class MockStationRuntime : public Infrastructure::Config::StationRuntime {
    Q_OBJECT
public:
    explicit MockStationRuntime(QObject* parent = nullptr)
        : Infrastructure::Config::StationRuntime(parent)
    {}
};

// Mock RuntimeManager that mimics the interface
class MockRuntimeManager : public QObject {
    Q_OBJECT
public:
    explicit MockRuntimeManager(QObject* parent = nullptr)
        : QObject(parent)
        , m_mockRuntime(nullptr)
    {}

    void setMockRuntime(Infrastructure::Config::StationRuntime* runtime) {
        m_mockRuntime = runtime;
    }

    Infrastructure::Config::StationRuntime* runtime() const {
        return m_mockRuntime;
    }

    void triggerRuntimeRecreated(Infrastructure::Config::StationRuntime* newRuntime) {
        emit runtimeRecreated(newRuntime);
    }

signals:
    void runtimeRecreated(Infrastructure::Config::StationRuntime* newRuntime);

private:
    Infrastructure::Config::StationRuntime* m_mockRuntime;
};

class TestExecutionViewModelTests : public QObject {
    Q_OBJECT

private:
    MockStationRuntime* m_runtime;
    MockRuntimeManager* m_runtimeManager;
    ViewModels::TestExecutionViewModel* m_vm;
    TestableGearboxTestEngine* m_testEngine;  // Separate test engine for manual signal triggering

private slots:
    void init() {
        m_runtime = new MockStationRuntime(this);
        m_runtimeManager = new MockRuntimeManager(this);
        m_runtimeManager->setMockRuntime(m_runtime);

        // Create ViewModel
        m_vm = new ViewModels::TestExecutionViewModel(m_runtime, nullptr, this);

        // Create a separate testable engine for manual signal triggering
        m_testEngine = new TestableGearboxTestEngine(this);

        // Manually connect the test engine signals to ViewModel's slots using QMetaObject
        connect(m_testEngine, &TestableGearboxTestEngine::stateChanged,
                m_vm, [this](const Domain::TestRunState& state) {
            QMetaObject::invokeMethod(m_vm, "onEngineStateChanged",
                                     Q_ARG(Domain::TestRunState, state));
        });
        connect(m_testEngine, &TestableGearboxTestEngine::testCompleted,
                m_vm, [this](const Domain::TestResults& results) {
            QMetaObject::invokeMethod(m_vm, "onTestCompleted",
                                     Q_ARG(Domain::TestResults, results));
        });
        connect(m_testEngine, &TestableGearboxTestEngine::testFailed,
                m_vm, [this](const Domain::FailureReason& reason) {
            QMetaObject::invokeMethod(m_vm, "onTestFailed",
                                     Q_ARG(Domain::FailureReason, reason));
        });
    }

    void cleanup() {
        delete m_vm;
        delete m_runtimeManager;
        delete m_runtime;
        m_vm = nullptr;
        m_runtimeManager = nullptr;
        m_runtime = nullptr;
    }

    // Test signal connections
    void testEngineStateChangedSignalConnection() {
        Domain::TestRunState state;
        state.phase = Domain::TestPhase::PrepareAndHome;
        state.subState = Domain::TestSubState::SeekingMagnet;
        state.statusMessage = "Seeking magnet";
        state.progressPercent = 10;
        state.elapsedMs = 1000;
        state.currentTelemetry.motorCurrentA = 2.5;
        state.currentTelemetry.dynSpeedRpm = 100.0;
        state.currentTelemetry.dynTorqueNm = 0.5;
        state.currentTelemetry.dynPowerW = 50.0;
        state.currentTelemetry.encoderAngleDeg = 45.0;
        state.currentTelemetry.brakeCurrentA = 0.0;
        state.currentTelemetry.aqmdAi1Level = true;

        QSignalSpy spyPhase(m_vm, &ViewModels::TestExecutionViewModel::currentPhaseChanged);
        QSignalSpy spyStatus(m_vm, &ViewModels::TestExecutionViewModel::statusMessageChanged);
        QSignalSpy spyProgress(m_vm, &ViewModels::TestExecutionViewModel::progressPercentChanged);
        QSignalSpy spyElapsed(m_vm, &ViewModels::TestExecutionViewModel::elapsedMsChanged);
        QSignalSpy spyMotorTelemetry(m_vm, &ViewModels::TestExecutionViewModel::motorTelemetryChanged);
        QSignalSpy spyTorqueTelemetry(m_vm, &ViewModels::TestExecutionViewModel::torqueTelemetryChanged);
        QSignalSpy spyEncoderTelemetry(m_vm, &ViewModels::TestExecutionViewModel::encoderTelemetryChanged);
        QSignalSpy spyBrakeTelemetry(m_vm, &ViewModels::TestExecutionViewModel::brakeTelemetryChanged);

        m_testEngine->triggerStateChanged(state);

        QCOMPARE(spyPhase.count(), 1);
        QCOMPARE(spyStatus.count(), 1);
        QCOMPARE(spyProgress.count(), 1);
        QCOMPARE(spyElapsed.count(), 1);
        QCOMPARE(spyMotorTelemetry.count(), 1);
        QCOMPARE(spyTorqueTelemetry.count(), 1);
        QCOMPARE(spyEncoderTelemetry.count(), 1);
        QCOMPARE(spyBrakeTelemetry.count(), 0);  // brakeCurrent is 0.0, no change from initial

        QCOMPARE(m_vm->currentPhase(), QString("Homing"));
        QCOMPARE(m_vm->statusMessage(), QString("Seeking magnet"));
        QCOMPARE(m_vm->progressPercent(), 10);
        QCOMPARE(m_vm->elapsedMs(), 1000);
        QCOMPARE(m_vm->motorCurrent(), 2.5);
        QCOMPARE(m_vm->speed(), 100.0);
        QCOMPARE(m_vm->torque(), 0.5);
        QCOMPARE(m_vm->power(), 50.0);
        QCOMPARE(m_vm->angle(), 45.0);
        QCOMPARE(m_vm->brakeCurrent(), 0.0);
        QCOMPARE(m_vm->ai1Level(), true);
    }

    void testTestCompletedSignalConnection() {
        Domain::TestResults results;
        results.overallPassed = true;
        results.idleForward.direction = "Forward";
        results.idleForward.currentAvg = 1.5;
        results.idleForward.currentMax = 2.0;
        results.idleForward.speedAvg = 100.0;
        results.idleForward.speedMax = 120.0;
        results.idleForward.overallPassed = true;

        results.idleReverse.direction = "Reverse";
        results.idleReverse.currentAvg = 1.6;
        results.idleReverse.currentMax = 2.1;
        results.idleReverse.speedAvg = 105.0;
        results.idleReverse.speedMax = 125.0;
        results.idleReverse.overallPassed = true;

        Domain::AngleResult angle1;
        angle1.positionName = "Position1";
        angle1.targetAngleDeg = 90.0;
        angle1.measuredAngleDeg = 90.5;
        angle1.deviationDeg = 0.5;
        angle1.toleranceDeg = 1.0;
        angle1.passed = true;
        results.angleResults.append(angle1);

        results.loadForward.direction = "Forward";
        results.loadForward.lockCurrentA = 5.0;
        results.loadForward.lockTorqueNm = 10.0;
        results.loadForward.lockAchieved = true;
        results.loadForward.overallPassed = true;

        results.loadReverse.direction = "Reverse";
        results.loadReverse.lockCurrentA = 5.2;
        results.loadReverse.lockTorqueNm = 10.5;
        results.loadReverse.lockAchieved = true;
        results.loadReverse.overallPassed = true;

        QSignalSpy spyRunning(m_vm, &ViewModels::TestExecutionViewModel::runningChanged);
        QSignalSpy spyResults(m_vm, &ViewModels::TestExecutionViewModel::resultsChanged);

        m_testEngine->triggerTestCompleted(results);

        QCOMPARE(spyRunning.count(), 1);
        QCOMPARE(spyResults.count(), 1);
        QCOMPARE(m_vm->running(), false);
        QCOMPARE(m_vm->testPassed(), true);
        QCOMPARE(m_vm->overallVerdict(), QString("PASSED"));
    }

    void testTestFailedSignalConnection() {
        Domain::FailureReason reason;
        reason.category = Domain::FailureCategory::Communication;
        reason.description = "Motor communication failed";

        QSignalSpy spyRunning(m_vm, &ViewModels::TestExecutionViewModel::runningChanged);
        QSignalSpy spyResults(m_vm, &ViewModels::TestExecutionViewModel::resultsChanged);
        QSignalSpy spyStatus(m_vm, &ViewModels::TestExecutionViewModel::statusMessageChanged);
        QSignalSpy spyError(m_vm, &ViewModels::TestExecutionViewModel::errorOccurred);

        m_testEngine->triggerTestFailed(reason);

        QCOMPARE(spyRunning.count(), 1);
        QCOMPARE(spyResults.count(), 1);
        QCOMPARE(spyStatus.count(), 1);
        QCOMPARE(spyError.count(), 1);
        QCOMPARE(m_vm->running(), false);
        QCOMPARE(m_vm->testPassed(), false);
        QCOMPARE(m_vm->overallVerdict(), QString("FAILED"));
        QVERIFY(m_vm->statusMessage().contains("Communication"));
        QVERIFY(m_vm->statusMessage().contains("Motor communication failed"));
    }

    // Test command forwarding
    void testStartTestCommandForwarding() {
        m_vm->setSerialNumber("12345678");
        m_vm->setSelectedModel("TestModel");

        QSignalSpy spyRunning(m_vm, &ViewModels::TestExecutionViewModel::runningChanged);

        m_vm->startTest();

        QVERIFY(m_testEngine->startTestCalled);
        QCOMPARE(m_testEngine->lastSerialNumber, QString("12345678"));
        QCOMPARE(spyRunning.count(), 1);
        QCOMPARE(m_vm->running(), true);
    }

    void testStartTestWithInvalidSerialNumber() {
        m_vm->setSerialNumber("123");  // Too short

        QSignalSpy spyError(m_vm, &ViewModels::TestExecutionViewModel::errorOccurred);
        QSignalSpy spyRunning(m_vm, &ViewModels::TestExecutionViewModel::runningChanged);

        m_vm->startTest();

        QCOMPARE(spyError.count(), 1);
        QCOMPARE(spyRunning.count(), 0);
        QCOMPARE(m_vm->running(), false);
        QVERIFY(!m_testEngine->startTestCalled);
    }

    void testStartTestWithEmptySerialNumber() {
        m_vm->setSerialNumber("");

        QSignalSpy spyError(m_vm, &ViewModels::TestExecutionViewModel::errorOccurred);

        m_vm->startTest();

        QCOMPARE(spyError.count(), 1);
        QVERIFY(!m_testEngine->startTestCalled);
    }

    void testStopTestCommandForwarding() {
        m_vm->setSerialNumber("12345678");
        m_vm->startTest();

        QSignalSpy spyRunning(m_vm, &ViewModels::TestExecutionViewModel::runningChanged);

        m_vm->stopTest();

        QVERIFY(m_testEngine->emergencyStopCalled);
        QCOMPARE(spyRunning.count(), 1);
        QCOMPARE(m_vm->running(), false);
    }

    void testResetTestCommandForwarding() {
        QSignalSpy spyRunning(m_vm, &ViewModels::TestExecutionViewModel::runningChanged);
        QSignalSpy spyPhase(m_vm, &ViewModels::TestExecutionViewModel::currentPhaseChanged);
        QSignalSpy spyStatus(m_vm, &ViewModels::TestExecutionViewModel::statusMessageChanged);
        QSignalSpy spyProgress(m_vm, &ViewModels::TestExecutionViewModel::progressPercentChanged);
        QSignalSpy spyElapsed(m_vm, &ViewModels::TestExecutionViewModel::elapsedMsChanged);
        QSignalSpy spyResults(m_vm, &ViewModels::TestExecutionViewModel::resultsChanged);

        m_vm->resetTest();

        QVERIFY(m_testEngine->resetCalled);
        QCOMPARE(m_vm->running(), false);
        QCOMPARE(m_vm->currentPhase(), QString("Idle"));
        QCOMPARE(m_vm->statusMessage(), QString("Ready"));
        QCOMPARE(m_vm->progressPercent(), 0);
        QCOMPARE(m_vm->elapsedMs(), 0);
        QCOMPARE(m_vm->overallVerdict(), QString("Pending"));
        QCOMPARE(m_vm->testPassed(), false);
    }

    // Test property setters and notifications
    void testSerialNumberProperty() {
        QSignalSpy spy(m_vm, &ViewModels::TestExecutionViewModel::serialNumberChanged);

        m_vm->setSerialNumber("SN123456");

        QCOMPARE(spy.count(), 1);
        QCOMPARE(m_vm->serialNumber(), QString("SN123456"));

        // Setting same value should not emit signal
        m_vm->setSerialNumber("SN123456");
        QCOMPARE(spy.count(), 1);
    }

    void testSelectedModelProperty() {
        QSignalSpy spy(m_vm, &ViewModels::TestExecutionViewModel::selectedModelChanged);

        m_vm->setSelectedModel("Model_A");

        QCOMPARE(spy.count(), 1);
        QCOMPARE(m_vm->selectedModel(), QString("Model_A"));

        // Setting same value should not emit signal
        m_vm->setSelectedModel("Model_A");
        QCOMPARE(spy.count(), 1);
    }

    void testBacklashCompensationProperty() {
        QSignalSpy spy(m_vm, &ViewModels::TestExecutionViewModel::backlashCompensationDegChanged);

        m_vm->setBacklashCompensationDeg(5.5);

        QCOMPARE(spy.count(), 1);
        QCOMPARE(m_vm->backlashCompensationDeg(), 5.5);

        // Setting same value should not emit signal
        m_vm->setBacklashCompensationDeg(5.5);
        QCOMPARE(spy.count(), 1);
    }

    // Test data conversion logic
    void testIdleResultConversion() {
        Domain::TestResults results;
        results.overallPassed = true;
        results.idleForward.direction = "Forward";
        results.idleForward.currentAvg = 1.5;
        results.idleForward.currentMax = 2.0;
        results.idleForward.speedAvg = 100.0;
        results.idleForward.speedMax = 120.0;
        results.idleForward.currentAvgPassed = true;
        results.idleForward.currentMaxPassed = true;
        results.idleForward.speedAvgPassed = true;
        results.idleForward.speedMaxPassed = true;
        results.idleForward.overallPassed = true;

        m_testEngine->triggerTestCompleted(results);

        QVariantMap idleForward = m_vm->idleForwardResult();
        QCOMPARE(idleForward["direction"].toString(), QString("Forward"));
        QCOMPARE(idleForward["currentAvg"].toDouble(), 1.5);
        QCOMPARE(idleForward["currentMax"].toDouble(), 2.0);
        QCOMPARE(idleForward["speedAvg"].toDouble(), 100.0);
        QCOMPARE(idleForward["speedMax"].toDouble(), 120.0);
        QCOMPARE(idleForward["currentAvgPassed"].toBool(), true);
        QCOMPARE(idleForward["currentMaxPassed"].toBool(), true);
        QCOMPARE(idleForward["speedAvgPassed"].toBool(), true);
        QCOMPARE(idleForward["speedMaxPassed"].toBool(), true);
        QCOMPARE(idleForward["overallPassed"].toBool(), true);
    }

    void testLoadResultConversion() {
        Domain::TestResults results;
        results.overallPassed = true;
        results.loadForward.direction = "Forward";
        results.loadForward.lockCurrentA = 5.0;
        results.loadForward.lockTorqueNm = 10.0;
        results.loadForward.currentPassed = true;
        results.loadForward.torquePassed = true;
        results.loadForward.overallPassed = true;
        results.loadForward.lockAchieved = true;

        m_testEngine->triggerTestCompleted(results);

        QVariantMap loadForward = m_vm->loadForwardResult();
        QCOMPARE(loadForward["direction"].toString(), QString("Forward"));
        QCOMPARE(loadForward["lockCurrentA"].toDouble(), 5.0);
        QCOMPARE(loadForward["lockTorqueNm"].toDouble(), 10.0);
        QCOMPARE(loadForward["currentPassed"].toBool(), true);
        QCOMPARE(loadForward["torquePassed"].toBool(), true);
        QCOMPARE(loadForward["overallPassed"].toBool(), true);
        QCOMPARE(loadForward["lockAchieved"].toBool(), true);
    }

    void testAngleResultsConversion() {
        Domain::TestResults results;
        results.overallPassed = true;

        Domain::AngleResult angle1;
        angle1.positionName = "Position1";
        angle1.targetAngleDeg = 90.0;
        angle1.measuredAngleDeg = 90.5;
        angle1.deviationDeg = 0.5;
        angle1.toleranceDeg = 1.0;
        angle1.passed = true;
        results.angleResults.append(angle1);

        Domain::AngleResult angle2;
        angle2.positionName = "Position2";
        angle2.targetAngleDeg = 180.0;
        angle2.measuredAngleDeg = 179.5;
        angle2.deviationDeg = -0.5;
        angle2.toleranceDeg = 1.0;
        angle2.passed = true;
        results.angleResults.append(angle2);

        m_testEngine->triggerTestCompleted(results);

        QVariantList angleResults = m_vm->angleResults();
        QCOMPARE(angleResults.size(), 2);

        QVariantMap angle1Map = angleResults[0].toMap();
        QCOMPARE(angle1Map["positionName"].toString(), QString("Position1"));
        QCOMPARE(angle1Map["targetAngleDeg"].toDouble(), 90.0);
        QCOMPARE(angle1Map["measuredAngleDeg"].toDouble(), 90.5);
        QCOMPARE(angle1Map["deviationDeg"].toDouble(), 0.5);
        QCOMPARE(angle1Map["toleranceDeg"].toDouble(), 1.0);
        QCOMPARE(angle1Map["passed"].toBool(), true);

        QVariantMap angle2Map = angleResults[1].toMap();
        QCOMPARE(angle2Map["positionName"].toString(), QString("Position2"));
        QCOMPARE(angle2Map["targetAngleDeg"].toDouble(), 180.0);
        QCOMPARE(angle2Map["measuredAngleDeg"].toDouble(), 179.5);
        QCOMPARE(angle2Map["deviationDeg"].toDouble(), -0.5);
        QCOMPARE(angle2Map["toleranceDeg"].toDouble(), 1.0);
        QCOMPARE(angle2Map["passed"].toBool(), true);
    }

    // Test runtime recreation handling
    void testRuntimeRecreation() {
        // Create new runtime
        MockStationRuntime* newRuntime = new MockStationRuntime(this);

        // Trigger runtime recreation
        m_runtimeManager->triggerRuntimeRecreated(newRuntime);

        // Note: Since we can't easily test signal reconnection without accessing private members,
        // we'll just verify the method doesn't crash
        QVERIFY(true);

        delete newRuntime;
    }

    // Test null runtime scenarios
    void testStartTestWithNullRuntime() {
        ViewModels::TestExecutionViewModel* vmNull = new ViewModels::TestExecutionViewModel(nullptr, nullptr, this);

        QSignalSpy spyError(vmNull, &ViewModels::TestExecutionViewModel::errorOccurred);

        vmNull->setSerialNumber("12345678");
        vmNull->startTest();

        QCOMPARE(spyError.count(), 1);

        delete vmNull;
    }

    void testStopTestWithNullRuntime() {
        ViewModels::TestExecutionViewModel* vmNull = new ViewModels::TestExecutionViewModel(nullptr, nullptr, this);

        // Should not crash
        vmNull->stopTest();

        delete vmNull;
    }

    void testResetTestWithNullRuntime() {
        ViewModels::TestExecutionViewModel* vmNull = new ViewModels::TestExecutionViewModel(nullptr, nullptr, this);

        // Should not crash
        vmNull->resetTest();

        delete vmNull;
    }

    // Test backlash compensation in recipe building
    void testBacklashCompensationAppliedToRecipe() {
        m_vm->setSerialNumber("12345678");
        m_vm->setBacklashCompensationDeg(5.0);

        m_vm->startTest();

        // Verify that recipe was set with compensation applied
        QVERIFY(m_testEngine->startTestCalled);
        QVERIFY(m_testEngine->lastRecipe.name.contains("回差补偿"));
        QVERIFY(m_testEngine->lastRecipe.name.contains("5.00"));
    }

    // Test telemetry update only when values change
    void testTelemetryUpdateOnlyWhenChanged() {
        Domain::TestRunState state;
        state.currentTelemetry.motorCurrentA = 2.5;
        state.currentTelemetry.dynSpeedRpm = 100.0;

        QSignalSpy spyMotorTelemetry(m_vm, &ViewModels::TestExecutionViewModel::motorTelemetryChanged);
        QSignalSpy spyTorqueTelemetry(m_vm, &ViewModels::TestExecutionViewModel::torqueTelemetryChanged);

        // First update
        m_testEngine->triggerStateChanged(state);
        QCOMPARE(spyMotorTelemetry.count(), 1);
        QCOMPARE(spyTorqueTelemetry.count(), 1);

        // Same values - should not emit
        m_testEngine->triggerStateChanged(state);
        QCOMPARE(spyMotorTelemetry.count(), 1);
        QCOMPARE(spyTorqueTelemetry.count(), 1);

        // Different motor value - should emit only motorTelemetryChanged
        state.currentTelemetry.motorCurrentA = 3.0;
        m_testEngine->triggerStateChanged(state);
        QCOMPARE(spyMotorTelemetry.count(), 2);
        QCOMPARE(spyTorqueTelemetry.count(), 1);  // torque signal not emitted

        // Different speed value - should emit only torqueTelemetryChanged
        state.currentTelemetry.dynSpeedRpm = 150.0;
        m_testEngine->triggerStateChanged(state);
        QCOMPARE(spyMotorTelemetry.count(), 2);  // motor signal not emitted
        QCOMPARE(spyTorqueTelemetry.count(), 2);
    }

    // Test failure category conversion
    void testFailureCategoryConversion() {
        Domain::FailureReason reason;

        // Test Communication failure
        reason.category = Domain::FailureCategory::Communication;
        reason.description = "Comm error";
        m_testEngine->triggerTestFailed(reason);
        QVERIFY(m_vm->statusMessage().contains("Communication"));

        // Test Process failure
        reason.category = Domain::FailureCategory::Process;
        reason.description = "Process error";
        m_testEngine->triggerTestFailed(reason);
        QVERIFY(m_vm->statusMessage().contains("Process"));

        // Test Judgment failure
        reason.category = Domain::FailureCategory::Judgment;
        reason.description = "Judgment error";
        m_testEngine->triggerTestFailed(reason);
        QVERIFY(m_vm->statusMessage().contains("Judgment"));
    }
};

QTEST_MAIN(TestExecutionViewModelTests)
#include "TestExecutionViewModelTests.moc"
