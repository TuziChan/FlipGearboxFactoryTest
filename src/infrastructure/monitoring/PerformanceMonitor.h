#ifndef PERFORMANCEMONITOR_H
#define PERFORMANCEMONITOR_H

#include <QObject>
#include <QElapsedTimer>
#include <QTimer>
#include <QMap>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <memory>

namespace Infrastructure {
namespace Monitoring {

/**
 * @brief Performance metric snapshot at a point in time
 */
struct PerformanceSnapshot {
    QDateTime timestamp;
    qint64 memoryUsedKb = 0;
    qint64 memoryPeakKb = 0;
    double cpuUsagePercent = 0.0;
    double frameRate = 0.0;
    qint64 eventLoopLatencyMs = 0;
    int activeTimers = 0;
    int activeThreads = 0;

    QJsonObject toJson() const;
};

/**
 * @brief Performance statistics for a named operation/phase
 */
struct OperationMetrics {
    QString operationName;
    qint64 totalCalls = 0;
    qint64 totalDurationMs = 0;
    qint64 minDurationMs = INT64_MAX;
    qint64 maxDurationMs = 0;
    double avgDurationMs = 0.0;

    void recordCall(qint64 durationMs);
    QJsonObject toJson() const;
};

/**
 * @brief Real-time performance monitoring service
 *
 * Monitors:
 * - Memory usage (working set)
 * - CPU utilization
 * - QML frame rate (if applicable)
 * - Event loop latency
 * - Named operation timing
 *
 * All methods are Q_INVOKABLE for QML access.
 * Results can be exported as JSON for test reports.
 */
class PerformanceMonitor : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool isRunning READ isRunning NOTIFY runningChanged)
    Q_PROPERTY(double currentFrameRate READ currentFrameRate NOTIFY frameRateChanged)
    Q_PROPERTY(qint64 currentMemoryKb READ currentMemoryKb NOTIFY memoryChanged)

public:
    explicit PerformanceMonitor(QObject* parent = nullptr);
    ~PerformanceMonitor() override;

    static PerformanceMonitor* instance();

    /**
     * @brief Start monitoring with specified interval
     */
    Q_INVOKABLE void startMonitoring(int intervalMs = 1000);

    /**
     * @brief Stop monitoring
     */
    Q_INVOKABLE void stopMonitoring();

    /**
     * @brief Check if monitoring is active
     */
    Q_INVOKABLE bool isRunning() const { return m_monitoringActive; }

    /**
     * @brief Take a single performance snapshot
     */
    Q_INVOKABLE PerformanceSnapshot takeSnapshot();

    /**
     * @brief Start timing a named operation
     */
    Q_INVOKABLE void startOperation(const QString& name);

    /**
     * @brief End timing a named operation
     */
    Q_INVOKABLE void endOperation(const QString& name);

    /**
     * @brief Get metrics for a named operation
     */
    Q_INVOKABLE OperationMetrics getOperationMetrics(const QString& name) const;

    /**
     * @brief Get all operation metrics
     */
    Q_INVOKABLE QMap<QString, OperationMetrics> allOperationMetrics() const;

    /**
     * @brief Get monitoring history
     */
    Q_INVOKABLE QVector<PerformanceSnapshot> history() const { return m_history; }

    /**
     * @brief Clear all history and metrics
     */
    Q_INVOKABLE void clear();

    /**
     * @brief Export monitoring data to JSON
     */
    Q_INVOKABLE bool exportToJson(const QString& filePath);

    /**
     * @brief Export monitoring data to HTML report
     */
    Q_INVOKABLE bool exportToHtml(const QString& filePath);

    /**
     * @brief Get current memory usage in KB
     */
    qint64 currentMemoryKb() const;

    /**
     * @brief Get current frame rate
     */
    double currentFrameRate() const { return m_currentFrameRate; }

    /**
     * @brief Set frame rate (called from rendering thread)
     */
    void updateFrameRate(double fps);

    /**
     * @brief Get peak memory usage during monitoring
     */
    qint64 peakMemoryKb() const { return m_peakMemoryKb; }

signals:
    void runningChanged(bool running);
    void frameRateChanged(double fps);
    void memoryChanged(qint64 memoryKb);
    void snapshotTaken(const PerformanceSnapshot& snapshot);
    void operationRecorded(const QString& name, qint64 durationMs);

private slots:
    void onMonitorTick();

private:
    static PerformanceMonitor* s_instance;

    bool m_monitoringActive = false;
    QTimer* m_monitorTimer = nullptr;
    QElapsedTimer m_operationTimer;
    QMap<QString, QElapsedTimer> m_activeOperations;
    QMap<QString, OperationMetrics> m_operationMetrics;
    QVector<PerformanceSnapshot> m_history;

    double m_currentFrameRate = 0.0;
    qint64 m_peakMemoryKb = 0;
    int m_frameCount = 0;
    QElapsedTimer m_frameTimer;

    qint64 queryMemoryUsageKb();
    double queryCpuUsagePercent();

    // CPU usage calculation state (cross-platform storage)
    quint64 m_lastCpuKernelTime = 0;
    quint64 m_lastCpuUserTime = 0;
    qint64 m_lastCpuWallTimeMs = 0;
};

/**
 * @brief RAII helper for automatic operation timing
 */
class ScopedPerformanceTimer {
public:
    explicit ScopedPerformanceTimer(const QString& operationName,
                                     PerformanceMonitor* monitor = nullptr);
    ~ScopedPerformanceTimer();

private:
    QString m_operationName;
    PerformanceMonitor* m_monitor;
    QElapsedTimer m_timer;
};

} // namespace Monitoring
} // namespace Infrastructure

#endif // PERFORMANCEMONITOR_H
