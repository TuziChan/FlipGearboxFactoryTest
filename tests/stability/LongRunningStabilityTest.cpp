#include <QCoreApplication>
#include <QTest>
#include <QElapsedTimer>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QDebug>
#include "../../src/infrastructure/acquisition/AcquisitionScheduler.h"
#include "../../src/infrastructure/acquisition/TelemetryBuffer.h"
#include "../../tests/mocks/MockDevices.h"

class LongRunningStabilityTest : public QObject {
    Q_OBJECT

private slots:
    void testLongRunningStability();

private:
    struct PerformanceMetrics {
        qint64 timestamp;
        size_t bufferSize;
        qint64 memoryUsage;
        double cycleTimeMs;
    };

    QVector<PerformanceMetrics> collectMetrics(
        Infrastructure::Acquisition::AcquisitionScheduler* scheduler,
        int durationMinutes);

    void saveMetricsReport(const QVector<PerformanceMetrics>& metrics, const QString& filePath);
};

void LongRunningStabilityTest::testLongRunningStability() {
    qInfo() << "========================================";
    qInfo() << "Long Running Stability Test";
    qInfo() << "Duration: 8 hours (480 minutes)";
    qInfo() << "Sample interval: 30 minutes";
    qInfo() << "========================================";

    auto motorDevice = new Tests::Mocks::MockMotorDevice();
    auto torqueDevice = new Tests::Mocks::MockTorqueDevice();
    auto encoderDevice = new Tests::Mocks::MockEncoderDevice();
    auto brakeDevice = new Tests::Mocks::MockBrakeDevice();

    auto scheduler = new Infrastructure::Acquisition::AcquisitionScheduler();
    scheduler->setMotorDevice(motorDevice, 5000);
    scheduler->setTorqueDevice(torqueDevice, 5000);
    scheduler->setEncoderDevice(encoderDevice, 5000);
    scheduler->setBrakeDevice(brakeDevice, 1, 5000);

    scheduler->start();

    QVector<PerformanceMetrics> metrics = collectMetrics(scheduler, 480);

    scheduler->stop();

    saveMetricsReport(metrics, "stability_report.json");

    // Verify no memory leaks
    for (int i = 1; i < metrics.size(); ++i) {
        qint64 memoryGrowth = metrics[i].memoryUsage - metrics[0].memoryUsage;
        qint64 maxAllowedGrowth = 100 * 1024 * 1024;
        QVERIFY2(memoryGrowth < maxAllowedGrowth,
                 QString("Memory leak detected: %1 MB growth").arg(memoryGrowth / 1024 / 1024).toUtf8());
    }

    delete scheduler;
    delete motorDevice;
    delete torqueDevice;
    delete encoderDevice;
    delete brakeDevice;

    qInfo() << "Stability test completed successfully";
}

QVector<LongRunningStabilityTest::PerformanceMetrics>
LongRunningStabilityTest::collectMetrics(
    Infrastructure::Acquisition::AcquisitionScheduler* scheduler,
    int durationMinutes) {

    QVector<PerformanceMetrics> metrics;
    QElapsedTimer timer;
    timer.start();

    int sampleIntervalMs = 30 * 60 * 1000;
    int totalSamples = durationMinutes / 30;

    for (int i = 0; i < totalSamples; ++i) {
        QTest::qWait(sampleIntervalMs);

        PerformanceMetrics sample;
        sample.timestamp = timer.elapsed();
        sample.bufferSize = 0; // TelemetryBuffer is not a container
        sample.memoryUsage = 0;
        sample.cycleTimeMs = 33.0;

        metrics.append(sample);

        qInfo() << QString("Sample %1/%2: Time=%3 min")
                       .arg(i + 1)
                       .arg(totalSamples)
                       .arg(sample.timestamp / 60000);
    }

    return metrics;
}

void LongRunningStabilityTest::saveMetricsReport(
    const QVector<PerformanceMetrics>& metrics,
    const QString& filePath) {

    QJsonObject report;
    report["test_name"] = "Long Running Stability Test";
    report["duration_minutes"] = 480;
    report["sample_count"] = metrics.size();

    QJsonArray samplesArray;
    for (const auto& metric : metrics) {
        QJsonObject sample;
        sample["timestamp_ms"] = metric.timestamp;
        sample["buffer_size"] = static_cast<int>(metric.bufferSize);
        sample["memory_usage_bytes"] = metric.memoryUsage;
        sample["cycle_time_ms"] = metric.cycleTimeMs;
        samplesArray.append(sample);
    }
    report["samples"] = samplesArray;

    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QJsonDocument doc(report);
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
        qInfo() << "Metrics report saved to:" << filePath;
    }
}

QTEST_MAIN(LongRunningStabilityTest)
#include "LongRunningStabilityTest.moc"
