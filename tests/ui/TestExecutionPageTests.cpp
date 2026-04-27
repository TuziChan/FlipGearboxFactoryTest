#include <QtTest>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWindow>
#include <QSignalSpy>
#include <memory>

#include "src/viewmodels/TestExecutionViewModel.h"
#include "src/infrastructure/config/RuntimeManager.h"
#include "src/infrastructure/config/StationConfig.h"
#include "src/infrastructure/config/StationRuntimeFactory.h"
#include "src/domain/GearboxTestEngine.h"
#include "src/domain/TestRecipe.h"

using namespace ViewModels;
using namespace Infrastructure::Config;
using namespace Domain;

class TestExecutionPageTests : public QObject
{
    Q_OBJECT

private:
    std::unique_ptr<RuntimeManager> m_runtimeManager;
    std::unique_ptr<TestExecutionViewModel> m_viewModel;
    QQmlApplicationEngine* m_engine = nullptr;
    QQuickWindow* m_window = nullptr;

private slots:
    void initTestCase() {
        qDebug() << "=== TestExecutionPage UI Tests ===";
    }

    void init() {
        // Create runtime manager in mock mode
        StationConfig config;
        config.aqmdConfig.portName = "COM1";
        config.aqmdConfig.baudRate = 9600;
        config.aqmdConfig.slaveId = 1;
        config.aqmdConfig.timeout = 1000;
        config.aqmdConfig.parity = "Even";
        config.aqmdConfig.stopBits = 1;

        m_runtimeManager = std::make_unique<RuntimeManager>(config, true);
        QVERIFY(m_runtimeManager->runtime() != nullptr);
        QVERIFY(m_runtimeManager->runtime()->isInitialized());

        // Create ViewModel
        m_viewModel = std::make_unique<TestExecutionViewModel>(
            m_runtimeManager->runtime(),
            m_runtimeManager.get()
        );
        QVERIFY(m_viewModel != nullptr);
    }

    void cleanup() {
        if (m_engine) {
            delete m_engine;
            m_engine = nullptr;
            m_window = nullptr;
        }
        m_viewModel.reset();
        m_runtimeManager.reset();
    }

    void cleanupTestCase() {
        qDebug() << "=== All tests completed ===";
    }

    // Test 1: Page loads successfully
    void testPageLoads() {
        m_engine = new QQmlApplicationEngine(this);
        m_engine->rootContext()->setContextProperty("testViewModel", m_viewModel.get());
        m_engine->rootContext()->setContextProperty("runtimeManager", m_runtimeManager.get());

        const QString qmlPath = QStringLiteral(TEST_SOURCE_DIR "/src/ui/pages/TestExecutionPage.qml");
        m_engine->load(QUrl::fromLocalFile(qmlPath));

        QVERIFY2(!m_engine->rootObjects().isEmpty(), "TestExecutionPage should load");

        auto* rootItem = qobject_cast<QQuickItem*>(m_engine->rootObjects().constFirst());
        QVERIFY2(rootItem != nullptr, "Root should be a QQuickItem");
        QCOMPARE(rootItem->objectName(), QStringLiteral("testExecutionPage"));
    }

    // Test 2: ViewModel binding works
    void testViewModelBinding() {
        m_engine = new QQmlApplicationEngine(this);
        m_engine->rootContext()->setContextProperty("testViewModel", m_viewModel.get());
        m_engine->rootContext()->setContextProperty("runtimeManager", m_runtimeManager.get());

        const QString qmlPath = QStringLiteral(TEST_SOURCE_DIR "/src/ui/pages/TestExecutionPage.qml");
        m_engine->load(QUrl::fromLocalFile(qmlPath));

        auto* rootItem = qobject_cast<QQuickItem*>(m_engine->rootObjects().constFirst());
        QVERIFY(rootItem != nullptr);

        // Check that ViewModel properties are accessible
        QVariant running = rootItem->property("running");
        QVERIFY(running.isValid());
        QCOMPARE(running.toBool(), false);

        QVariant phaseTitle = rootItem->property("phaseTitle");
        QVERIFY(phaseTitle.isValid());
        QVERIFY(!phaseTitle.toString().isEmpty());
    }

    // Test 3: Signal response - running state change
    void testRunningStateChange() {
        m_engine = new QQmlApplicationEngine(this);
        m_engine->rootContext()->setContextProperty("testViewModel", m_viewModel.get());
        m_engine->rootContext()->setContextProperty("runtimeManager", m_runtimeManager.get());

        const QString qmlPath = QStringLiteral(TEST_SOURCE_DIR "/src/ui/pages/TestExecutionPage.qml");
        m_engine->load(QUrl::fromLocalFile(qmlPath));

        auto* rootItem = qobject_cast<QQuickItem*>(m_engine->rootObjects().constFirst());
        QVERIFY(rootItem != nullptr);

        // Load a recipe via loadRecipe method
        m_viewModel->loadRecipe("default");

        // Start test
        QSignalSpy runningSpy(m_viewModel.get(), &TestExecutionViewModel::runningChanged);
        m_viewModel->startTest();

        QVERIFY(runningSpy.wait(100));
        QCOMPARE(m_viewModel->running(), true);

        // Check UI reflects the change
        QVariant running = rootItem->property("running");
        QCOMPARE(running.toBool(), true);
    }

    // Test 4: Test lifecycle - pause/resume
    void testPauseResume() {
        // Load recipe via loadRecipe method
        m_viewModel->loadRecipe("default");

        // Start test
        m_viewModel->startTest();
        QCOMPARE(m_viewModel->running(), true);

        // Stop test (pause)
        m_viewModel->stopTest();
        QCOMPARE(m_viewModel->running(), false);

        // Resume (start again)
        m_viewModel->startTest();
        QCOMPARE(m_viewModel->running(), true);
    }

    // Test 5: Test lifecycle - cancel mid-test
    void testCancelMidTest() {
        // Load recipe via loadRecipe method
        m_viewModel->loadRecipe("default");

        // Start test
        m_viewModel->startTest();
        QCOMPARE(m_viewModel->running(), true);

        // Wait a bit
        QTest::qWait(100);

        // Cancel
        m_viewModel->stopTest();
        QCOMPARE(m_viewModel->running(), false);
    }

    // Test 6: Test lifecycle - timeout interrupt
    void testTimeoutInterrupt() {
        // Load recipe with very short timeout
        m_viewModel->loadRecipe("default");

        QSignalSpy errorSpy(m_viewModel.get(), &TestExecutionViewModel::errorOccurred);

        // Start test
        m_viewModel->startTest();
        QCOMPARE(m_viewModel->running(), true);

        // Wait for timeout
        QVERIFY(errorSpy.wait(5000));

        // Test should stop due to timeout
        QCOMPARE(m_viewModel->running(), false);
    }

    // Test 7: Complete data flow UI → ViewModel → Engine
    void testCompleteDataFlow() {
        m_engine = new QQmlApplicationEngine(this);
        m_engine->rootContext()->setContextProperty("testViewModel", m_viewModel.get());
        m_engine->rootContext()->setContextProperty("runtimeManager", m_runtimeManager.get());

        const QString qmlPath = QStringLiteral(TEST_SOURCE_DIR "/src/ui/pages/TestExecutionPage.qml");
        m_engine->load(QUrl::fromLocalFile(qmlPath));

        auto* rootItem = qobject_cast<QQuickItem*>(m_engine->rootObjects().constFirst());
        QVERIFY(rootItem != nullptr);

        // Load recipe
        m_viewModel->loadRecipe("default");

        // Trigger start from UI (simulate button click)
        QMetaObject::invokeMethod(rootItem, "startRun");

        // Verify ViewModel received the command
        QCOMPARE(m_viewModel->running(), true);

        // Verify Engine state (use currentState instead of isRunning)
        auto* engine = m_runtimeManager->runtime()->testEngine();
        QVERIFY(engine != nullptr);
        QVERIFY(engine->currentState().phase != Domain::TestPhase::Idle);
    }
};

QTEST_MAIN(TestExecutionPageTests)
#include "TestExecutionPageTests.moc"
