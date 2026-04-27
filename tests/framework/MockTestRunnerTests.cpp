#include <QtTest>
#include "MockTestRunner.h"

using namespace Tests::Framework;
using namespace Infrastructure::Simulation;

/**
 * @brief Tests for the MockTestRunner framework
 */
class MockTestRunnerTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testMockEnvironmentInitialization();
    void testPredefinedScenarios();
    void testNormalOperationScenario();
    void testAngleTestWithMagnetDetection();
    void testStressTestScenario();
    void testEnvironmentFactory();
    void testScenarioSignals();
};

void MockTestRunnerTests::initTestCase()
{
    qDebug() << "========================================";
    qDebug() << "MockTestRunner Tests";
    qDebug() << "========================================";
}

void MockTestRunnerTests::cleanupTestCase()
{
    qDebug() << "MockTestRunner Tests Complete";
}

void MockTestRunnerTests::testMockEnvironmentInitialization()
{
    MockTestRunner runner;
    QVERIFY(runner.initializeMockEnvironment());
    QVERIFY(runner.mockDeviceManager() != nullptr);
    QVERIFY(runner.simulationContext() != nullptr);

    // Verify devices exist
    QVERIFY(runner.mockDeviceManager()->motorDevice() != nullptr);
    QVERIFY(runner.mockDeviceManager()->torqueDevice() != nullptr);
    QVERIFY(runner.mockDeviceManager()->encoderDevice() != nullptr);
    QVERIFY(runner.mockDeviceManager()->brakeDevice() != nullptr);
}

void MockTestRunnerTests::testPredefinedScenarios()
{
    auto scenarios = MockTestRunner::predefinedScenarios();
    QVERIFY(!scenarios.isEmpty());

    QStringList expectedNames = {
        "Normal Operation",
        "Error Injection",
        "Stress Test",
        "Angle Test with Magnet Detection",
        "Motor Load Simulation"
    };

    for (const auto& expected : expectedNames) {
        bool found = false;
        for (const auto& scenario : scenarios) {
            if (scenario.name == expected) {
                found = true;
                break;
            }
        }
        QVERIFY2(found, qPrintable(QString("Expected scenario not found: %1").arg(expected)));
    }
}

void MockTestRunnerTests::testNormalOperationScenario()
{
    MockTestRunner runner;
    QVERIFY(runner.initializeMockEnvironment());

    MockTestScenario scenario;
    scenario.name = "Normal Operation";
    scenario.description = "Basic operation test";

    QVERIFY(runner.runScenario(scenario));
}

void MockTestRunnerTests::testAngleTestWithMagnetDetection()
{
    MockTestRunner runner;
    QVERIFY(runner.initializeMockEnvironment());

    MockTestScenario scenario;
    scenario.name = "Angle Test with Magnet Detection";

    QVERIFY(runner.runScenario(scenario));
}

void MockTestRunnerTests::testStressTestScenario()
{
    MockTestRunner runner;
    QVERIFY(runner.initializeMockEnvironment());

    MockTestScenario scenario;
    scenario.name = "Stress Test";
    scenario.simulateHighSpeed = true;
    scenario.highSpeedDurationMs = 500;
    scenario.highSpeedRateHz = 100;

    QVERIFY(runner.runScenario(scenario));
}

void MockTestRunnerTests::testEnvironmentFactory()
{
    auto env = MockTestEnvironmentFactory::createEnvironment();
    QVERIFY(env != nullptr);
    QVERIFY(env->mockDeviceManager() != nullptr);

    MockTestScenario scenario;
    scenario.name = "Normal Operation";
    auto env2 = MockTestEnvironmentFactory::createEnvironment(scenario);
    QVERIFY(env2 != nullptr);
}

void MockTestRunnerTests::testScenarioSignals()
{
    MockTestRunner runner;
    QVERIFY(runner.initializeMockEnvironment());

    QSignalSpy startedSpy(&runner, &MockTestRunner::scenarioStarted);
    QSignalSpy finishedSpy(&runner, &MockTestRunner::scenarioFinished);

    MockTestScenario scenario;
    scenario.name = "Normal Operation";
    QVERIFY(runner.runScenario(scenario));

    QCOMPARE(startedSpy.count(), 1);
    QCOMPARE(finishedSpy.count(), 1);

    QList<QVariant> finishedArgs = finishedSpy.takeFirst();
    QCOMPARE(finishedArgs[0].toString(), QString("Normal Operation"));
    QVERIFY(finishedArgs[1].toBool());
    QVERIFY(finishedArgs[2].toLongLong() >= 0);
}

QTEST_MAIN(MockTestRunnerTests)
#include "MockTestRunnerTests.moc"
