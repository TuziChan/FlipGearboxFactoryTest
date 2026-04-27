#include <QCoreApplication>
#include <QDebug>
#include <QElapsedTimer>
#include <QThread>
#include "src/infrastructure/bus/ModbusRtuBusController.h"

void testEncoderStability(int intervalMs, int testDurationSec) {
    qDebug() << "\n========================================";
    qDebug() << "Testing at" << intervalMs << "ms interval for" << testDurationSec << "seconds";
    qDebug() << "========================================";

    Infrastructure::Bus::ModbusRtuBusController controller;

    if (!controller.open("COM23", 115200, 500, "None", 1)) {
        qCritical() << "Failed to open COM23:" << controller.lastError();
        return;
    }

    // Modbus request: Read holding register 0x0000 (1 register)
    QByteArray request;
    request.append(static_cast<char>(0x01)); // Slave ID
    request.append(static_cast<char>(0x03)); // Function code
    request.append(static_cast<char>(0x00)); // Start address high
    request.append(static_cast<char>(0x00)); // Start address low
    request.append(static_cast<char>(0x00)); // Quantity high
    request.append(static_cast<char>(0x01)); // Quantity low
    request.append(static_cast<char>(0x84)); // CRC low
    request.append(static_cast<char>(0x0A)); // CRC high

    int successCount = 0;
    int failCount = 0;
    int totalRequests = 0;
    QElapsedTimer testTimer;
    testTimer.start();

    qint64 minLatency = INT64_MAX;
    qint64 maxLatency = 0;
    qint64 totalLatency = 0;

    while (testTimer.elapsed() < testDurationSec * 1000) {
        QElapsedTimer requestTimer;
        requestTimer.start();

        QByteArray response;
        bool success = controller.sendRequest(request, response);

        qint64 latency = requestTimer.elapsed();
        totalRequests++;

        if (success) {
            successCount++;
            minLatency = qMin(minLatency, latency);
            maxLatency = qMax(maxLatency, latency);
            totalLatency += latency;
        } else {
            failCount++;
            qWarning() << "[" << totalRequests << "] FAILED:" << controller.lastError()
                       << "after" << latency << "ms";
        }

        // Sleep for the specified interval
        if (intervalMs > 0) {
            QThread::msleep(intervalMs);
        }
    }

    controller.close();

    // Print statistics
    qDebug() << "\n--- Test Results ---";
    qDebug() << "Total requests:" << totalRequests;
    qDebug() << "Success:" << successCount << QString("(%1%)").arg(successCount * 100.0 / totalRequests, 0, 'f', 2);
    qDebug() << "Failed:" << failCount << QString("(%1%)").arg(failCount * 100.0 / totalRequests, 0, 'f', 2);

    if (successCount > 0) {
        qDebug() << "Latency - Min:" << minLatency << "ms, Max:" << maxLatency << "ms, Avg:"
                 << (totalLatency / successCount) << "ms";
    }

    qDebug() << "Effective rate:" << (totalRequests * 1000.0 / testTimer.elapsed()) << "req/sec";
    qDebug() << "========================================\n";
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "=== Encoder Stability Test ===";
    qDebug() << "Testing different polling intervals to find optimal rate";
    qDebug() << "Main thread ID:" << QThread::currentThreadId();

    // Test different intervals
    testEncoderStability(100, 10);  // 100ms interval (10 Hz) - 10 seconds
    testEncoderStability(50, 10);   // 50ms interval (20 Hz) - 10 seconds
    testEncoderStability(20, 10);   // 20ms interval (50 Hz) - 10 seconds
    testEncoderStability(10, 10);   // 10ms interval (100 Hz) - 10 seconds
    testEncoderStability(5, 10);    // 5ms interval (200 Hz) - 10 seconds
    testEncoderStability(2, 10);    // 2ms interval (500 Hz) - 10 seconds
    testEncoderStability(1, 10);    // 1ms interval (1000 Hz) - 10 seconds
    testEncoderStability(0, 5);     // No delay (max speed) - 5 seconds

    qDebug() << "\n=== Test Complete ===";
    qDebug() << "Recommendation: Use the highest interval with <1% packet loss";

    return 0;
}
