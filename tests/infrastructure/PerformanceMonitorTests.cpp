#include <QtTest>
#include "../../src/infrastructure/monitoring/PerformanceMonitor.h"

using namespace Infrastructure::Monitoring;

/**
 * @brief Tests for the PerformanceMonitor service
 */
class PerformanceMonitorTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testSingleton();
    void testStartStopMonitoring();
    void testTakeSnapshot();
    void testMemoryMeasurement();
    void testOperationTiming();
    void testHistoryCollection();
    void testExportJson();
    void testExportHtml();
    void testScopedTimer();
};

void PerformanceMonitorTests::initTestCase()
{
    qDebug() << "========================================";
    qDebug() << "PerformanceMonitor Tests";
    qDebug() << "========================================";
}

void PerformanceMonitorTests::cleanupTestCase()
{
    qDebug() << "PerformanceMonitor Tests Complete";
}

void PerformanceMonitorTests::testSingleton()
{
    PerformanceMonitor* monitor1 = PerformanceMonitor::instance();
    PerformanceMonitor* monitor2 = PerformanceMonitor::instance();
    QCOMPARE(monitor1, monitor2);
    QVERIFY(monitor1 != nullptr);
}

void PerformanceMonitorTests::testStartStopMonitoring()
{
    PerformanceMonitor monitor;
    QVERIFY(!monitor.isRunning());

    QSignalSpy spy(&monitor, &PerformanceMonitor::runningChanged);
    monitor.startMonitoring(100);
    QVERIFY(monitor.isRunning());
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.takeFirst()[0].toBool());

    monitor.stopMonitoring();
    QVERIFY(!monitor.isRunning());
    QCOMPARE(spy.count(), 1);
    QVERIFY(!spy.takeFirst()[0].toBool());
}

void PerformanceMonitorTests::testTakeSnapshot()
{
    PerformanceMonitor monitor;
    auto snapshot = monitor.takeSnapshot();

    QVERIFY(snapshot.timestamp.isValid());
    QVERIFY(snapshot.memoryUsedKb > 0);
}

void PerformanceMonitorTests::testMemoryMeasurement()
{
    PerformanceMonitor monitor;
    qint64 mem1 = monitor.currentMemoryKb();
    QVERIFY(mem1 > 0);

    // Allocate some memory to see change
    QByteArray buffer(1024 * 1024, 'x'); // 1MB
    Q_UNUSED(buffer)

    qint64 mem2 = monitor.currentMemoryKb();
    QVERIFY(mem2 > 0);
}

void PerformanceMonitorTests::testOperationTiming()
{
    PerformanceMonitor monitor;

    monitor.startOperation("test_op");
    QTest::qSleep(50);
    monitor.endOperation("test_op");

    auto metrics = monitor.getOperationMetrics("test_op");
    QCOMPARE(metrics.operationName, QString("test_op"));
    QCOMPARE(metrics.totalCalls, 1);
    QVERIFY(metrics.avgDurationMs >= 50.0);
    QVERIFY(metrics.maxDurationMs >= 50);

    // Multiple calls
    monitor.startOperation("test_op");
    QTest::qSleep(20);
    monitor.endOperation("test_op");

    metrics = monitor.getOperationMetrics("test_op");
    QCOMPARE(metrics.totalCalls, 2);
}

void PerformanceMonitorTests::testHistoryCollection()
{
    PerformanceMonitor monitor;
    monitor.startMonitoring(50);

    QTest::qWait(200); // Allow a few ticks

    monitor.stopMonitoring();
    auto history = monitor.history();
    QVERIFY(history.size() >= 2);
}

void PerformanceMonitorTests::testExportJson()
{
    PerformanceMonitor monitor;
    monitor.takeSnapshot();

    monitor.startOperation("op1");
    monitor.endOperation("op1");

    QString tempFile = QDir::temp().filePath("perf_export.json");
    QVERIFY(monitor.exportToJson(tempFile));
    QVERIFY(QFile::exists(tempFile));

    QFile file(tempFile);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    QVERIFY(!doc.isNull());
    QVERIFY(doc.object().contains("history"));
    QVERIFY(doc.object().contains("operations"));

    QFile::remove(tempFile);
}

void PerformanceMonitorTests::testExportHtml()
{
    PerformanceMonitor monitor;
    monitor.takeSnapshot();

    monitor.startOperation("render");
    QTest::qSleep(10);
    monitor.endOperation("render");

    QString tempFile = QDir::temp().filePath("perf_export.html");
    QVERIFY(monitor.exportToHtml(tempFile));
    QVERIFY(QFile::exists(tempFile));

    QFile file(tempFile);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QString html = QString::fromUtf8(file.readAll());
    file.close();

    QVERIFY(html.contains("Performance Monitor Report"));
    QVERIFY(html.contains("render"));

    QFile::remove(tempFile);
}

void PerformanceMonitorTests::testScopedTimer()
{
    PerformanceMonitor monitor;

    {
        ScopedPerformanceTimer timer("scoped_op", &monitor);
        QTest::qSleep(30);
    }

    auto metrics = monitor.getOperationMetrics("scoped_op");
    QCOMPARE(metrics.totalCalls, 1);
    QVERIFY(metrics.avgDurationMs >= 30.0);
}

QTEST_MAIN(PerformanceMonitorTests)
#include "PerformanceMonitorTests.moc"
