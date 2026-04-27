#include "PerformanceMonitor.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTextStream>
#include <QDebug>
#include <QThread>

#ifdef Q_OS_WIN
#include <windows.h>
#include <psapi.h>
#endif

namespace Infrastructure {
namespace Monitoring {

// ============================================================================
// PerformanceSnapshot
// ============================================================================

QJsonObject PerformanceSnapshot::toJson() const {
    QJsonObject obj;
    obj["timestamp"] = timestamp.toString(Qt::ISODate);
    obj["memory_used_kb"] = static_cast<int>(memoryUsedKb);
    obj["memory_peak_kb"] = static_cast<int>(memoryPeakKb);
    obj["cpu_usage_percent"] = cpuUsagePercent;
    obj["frame_rate"] = frameRate;
    obj["event_loop_latency_ms"] = static_cast<int>(eventLoopLatencyMs);
    obj["active_timers"] = activeTimers;
    obj["active_threads"] = activeThreads;
    return obj;
}

// ============================================================================
// OperationMetrics
// ============================================================================

void OperationMetrics::recordCall(qint64 durationMs) {
    totalCalls++;
    totalDurationMs += durationMs;
    if (durationMs < minDurationMs) minDurationMs = durationMs;
    if (durationMs > maxDurationMs) maxDurationMs = durationMs;
    avgDurationMs = static_cast<double>(totalDurationMs) / totalCalls;
}

QJsonObject OperationMetrics::toJson() const {
    QJsonObject obj;
    obj["operation_name"] = operationName;
    obj["total_calls"] = static_cast<int>(totalCalls);
    obj["total_duration_ms"] = static_cast<int>(totalDurationMs);
    obj["min_duration_ms"] = static_cast<int>(minDurationMs);
    obj["max_duration_ms"] = static_cast<int>(maxDurationMs);
    obj["avg_duration_ms"] = avgDurationMs;
    return obj;
}

// ============================================================================
// PerformanceMonitor
// ============================================================================

PerformanceMonitor* PerformanceMonitor::s_instance = nullptr;

PerformanceMonitor::PerformanceMonitor(QObject* parent)
    : QObject(parent)
    , m_monitorTimer(new QTimer(this))
{
    connect(m_monitorTimer, &QTimer::timeout, this, &PerformanceMonitor::onMonitorTick);
    m_frameTimer.start();
}

PerformanceMonitor::~PerformanceMonitor() = default;

PerformanceMonitor* PerformanceMonitor::instance()
{
    if (!s_instance) {
        s_instance = new PerformanceMonitor();
    }
    return s_instance;
}

void PerformanceMonitor::startMonitoring(int intervalMs)
{
    if (m_monitoringActive) {
        return;
    }

    m_monitoringActive = true;
    m_peakMemoryKb = 0;
    m_history.clear();

    m_monitorTimer->start(intervalMs);
    emit runningChanged(true);

    qInfo() << "PerformanceMonitor: Started with interval" << intervalMs << "ms";
}

void PerformanceMonitor::stopMonitoring()
{
    if (!m_monitoringActive) {
        return;
    }

    m_monitoringActive = false;
    m_monitorTimer->stop();
    emit runningChanged(false);

    qInfo() << "PerformanceMonitor: Stopped. Snapshots recorded:" << m_history.size();
}

PerformanceSnapshot PerformanceMonitor::takeSnapshot()
{
    PerformanceSnapshot snapshot;
    snapshot.timestamp = QDateTime::currentDateTime();
    snapshot.memoryUsedKb = queryMemoryUsageKb();
    snapshot.memoryPeakKb = m_peakMemoryKb;
    snapshot.cpuUsagePercent = queryCpuUsagePercent();
    snapshot.frameRate = m_currentFrameRate;
    auto* dispatcher = QAbstractEventDispatcher::instance();
    snapshot.activeTimers = dispatcher ? static_cast<int>(dispatcher->registeredTimers(this).size()) : 0;
    snapshot.activeThreads = QThread::idealThreadCount();

    if (snapshot.memoryUsedKb > m_peakMemoryKb) {
        m_peakMemoryKb = snapshot.memoryUsedKb;
    }

    return snapshot;
}

void PerformanceMonitor::startOperation(const QString& name)
{
    QElapsedTimer timer;
    timer.start();
    m_activeOperations[name] = timer;
}

void PerformanceMonitor::endOperation(const QString& name)
{
    if (!m_activeOperations.contains(name)) {
        qWarning() << "PerformanceMonitor: No active operation named" << name;
        return;
    }

    qint64 durationMs = m_activeOperations[name].elapsed();
    m_activeOperations.remove(name);

    if (!m_operationMetrics.contains(name)) {
        OperationMetrics metrics;
        metrics.operationName = name;
        m_operationMetrics[name] = metrics;
    }

    m_operationMetrics[name].recordCall(durationMs);
    emit operationRecorded(name, durationMs);
}

OperationMetrics PerformanceMonitor::getOperationMetrics(const QString& name) const
{
    if (m_operationMetrics.contains(name)) {
        return m_operationMetrics.value(name);
    }
    return OperationMetrics();
}

QMap<QString, OperationMetrics> PerformanceMonitor::allOperationMetrics() const
{
    return m_operationMetrics;
}

void PerformanceMonitor::clear()
{
    m_history.clear();
    m_operationMetrics.clear();
    m_activeOperations.clear();
    m_peakMemoryKb = 0;
    m_currentFrameRate = 0.0;
}

bool PerformanceMonitor::exportToJson(const QString& filePath)
{
    QJsonObject root;
    root["export_time"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    root["monitoring_active"] = m_monitoringActive;
    root["peak_memory_kb"] = static_cast<int>(m_peakMemoryKb);

    QJsonArray historyArray;
    for (const auto& snapshot : m_history) {
        historyArray.append(snapshot.toJson());
    }
    root["history"] = historyArray;

    QJsonArray operationsArray;
    for (const auto& metrics : m_operationMetrics) {
        operationsArray.append(metrics.toJson());
    }
    root["operations"] = operationsArray;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "PerformanceMonitor: Failed to open file for export:" << filePath;
        return false;
    }

    QJsonDocument doc(root);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    qInfo() << "PerformanceMonitor: Exported to" << filePath;
    return true;
}

bool PerformanceMonitor::exportToHtml(const QString& filePath)
{
    QString html;
    html += "<!DOCTYPE html>\n<html><head><meta charset=\"UTF-8\">\n";
    html += "<title>Performance Monitor Report</title>\n";
    html += "<style>\n";
    html += "body{font-family:system-ui,sans-serif;margin:20px;background:#f5f5f5;}\n";
    html += ".container{max-width:1200px;margin:0 auto;background:#fff;padding:20px;border-radius:8px;}\n";
    html += "h1{color:#333;border-bottom:2px solid #e0e0e0;padding-bottom:10px;}\n";
    html += "table{width:100%;border-collapse:collapse;margin:15px 0;}\n";
    html += "th,td{padding:10px;text-align:left;border-bottom:1px solid #e0e0e0;}\n";
    html += "th{background:#f8f9fa;font-weight:600;}\n";
    html += ".metric-card{display:inline-block;background:#e3f2fd;padding:15px 20px;margin:5px;border-radius:6px;}\n";
    html += ".metric-value{font-size:24px;font-weight:bold;color:#1976d2;}\n";
    html += ".metric-label{font-size:12px;color:#666;text-transform:uppercase;}\n";
    html += "</style></head><body>\n";
    html += "<div class=\"container\">\n";
    html += "<h1>Performance Monitor Report</h1>\n";
    html += "<p>Generated: " + QDateTime::currentDateTime().toString(Qt::ISODate) + "</p>\n";

    // Summary metrics
    html += "<h2>Summary</h2>\n";
    html += "<div class=\"metric-card\">\n";
    html += "<div class=\"metric-value\">" + QString::number(m_history.size()) + "</div>\n";
    html += "<div class=\"metric-label\">Snapshots</div>\n";
    html += "</div>\n";
    html += "<div class=\"metric-card\">\n";
    html += "<div class=\"metric-value\">" + QString::number(m_peakMemoryKb) + " KB</div>\n";
    html += "<div class=\"metric-label\">Peak Memory</div>\n";
    html += "</div>\n";
    html += "<div class=\"metric-card\">\n";
    html += "<div class=\"metric-value\">" + QString::number(m_operationMetrics.size()) + "</div>\n";
    html += "<div class=\"metric-label\">Operations</div>\n";
    html += "</div>\n";

    // Operation metrics table
    if (!m_operationMetrics.isEmpty()) {
        html += "<h2>Operation Metrics</h2>\n";
        html += "<table>\n";
        html += "<tr><th>Operation</th><th>Calls</th><th>Total (ms)</th><th>Avg (ms)</th><th>Min (ms)</th><th>Max (ms)</th></tr>\n";
        for (const auto& metrics : m_operationMetrics) {
            html += "<tr>";
            html += "<td>" + metrics.operationName + "</td>";
            html += "<td>" + QString::number(metrics.totalCalls) + "</td>";
            html += "<td>" + QString::number(metrics.totalDurationMs) + "</td>";
            html += "<td>" + QString::number(metrics.avgDurationMs, 'f', 2) + "</td>";
            html += "<td>" + QString::number(metrics.minDurationMs) + "</td>";
            html += "<td>" + QString::number(metrics.maxDurationMs) + "</td>";
            html += "</tr>\n";
        }
        html += "</table>\n";
    }

    // History table
    if (!m_history.isEmpty()) {
        html += "<h2>Memory History</h2>\n";
        html += "<table>\n";
        html += "<tr><th>Timestamp</th><th>Memory (KB)</th><th>CPU %</th><th>FPS</th></tr>\n";
        for (const auto& snapshot : m_history) {
            html += "<tr>";
            html += "<td>" + snapshot.timestamp.toString(Qt::ISODate) + "</td>";
            html += "<td>" + QString::number(snapshot.memoryUsedKb) + "</td>";
            html += "<td>" + QString::number(snapshot.cpuUsagePercent, 'f', 1) + "</td>";
            html += "<td>" + QString::number(snapshot.frameRate, 'f', 1) + "</td>";
            html += "</tr>\n";
        }
        html += "</table>\n";
    }

    html += "</div></body></html>\n";

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "PerformanceMonitor: Failed to open HTML file:" << filePath;
        return false;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    stream << html;
    file.close();

    qInfo() << "PerformanceMonitor: HTML report exported to" << filePath;
    return true;
}

qint64 PerformanceMonitor::currentMemoryKb() const
{
    return const_cast<PerformanceMonitor*>(this)->queryMemoryUsageKb();
}

void PerformanceMonitor::updateFrameRate(double fps)
{
    m_currentFrameRate = fps;
    emit frameRateChanged(fps);
}

void PerformanceMonitor::onMonitorTick()
{
    auto snapshot = takeSnapshot();
    m_history.append(snapshot);

    emit memoryChanged(snapshot.memoryUsedKb);
    emit snapshotTaken(snapshot);

    // Keep history size manageable (max 1000 snapshots)
    if (m_history.size() > 1000) {
        m_history.removeFirst();
    }
}

qint64 PerformanceMonitor::queryMemoryUsageKb()
{
#ifdef Q_OS_WIN
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return static_cast<qint64>(pmc.WorkingSetSize / 1024);
    }
#else
    // Linux/macOS fallback
    QFile file("/proc/self/status");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QByteArray data = file.readAll();
        QRegularExpression re("VmRSS:\\s*(\\d+)\\s*kB");
        QRegularExpressionMatch match = re.match(data);
        if (match.hasMatch()) {
            return match.captured(1).toLongLong();
        }
    }
#endif
    return -1;
}

double PerformanceMonitor::queryCpuUsagePercent()
{
#ifdef Q_OS_WIN
    FILETIME creationTime, exitTime, kernelTime, userTime;
    if (GetProcessTimes(GetCurrentProcess(), &creationTime, &exitTime, &kernelTime, &userTime)) {
        auto fileTimeToUint64 = [](const FILETIME& ft) -> quint64 {
            return (static_cast<quint64>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
        };

        quint64 kernel = fileTimeToUint64(kernelTime);
        quint64 user = fileTimeToUint64(userTime);
        qint64 now = QDateTime::currentDateTime().toMSecsSinceEpoch();

        if (m_lastCpuWallTimeMs > 0) {
            quint64 kernelDelta = kernel - m_lastCpuKernelTime;
            quint64 userDelta = user - m_lastCpuUserTime;
            qint64 wallDelta = now - m_lastCpuWallTimeMs;

            if (wallDelta > 0) {
                // FILETIME unit is 100-nanosecond intervals
                double cpuTimeMs = static_cast<double>(kernelDelta + userDelta) / 10000.0;
                double cpuPercent = (cpuTimeMs / wallDelta) * 100.0;

                // Normalize by processor count to get 0-100% range
                int numProcessors = QThread::idealThreadCount();
                if (numProcessors > 1) {
                    cpuPercent /= numProcessors;
                }

                m_lastCpuKernelTime = kernel;
                m_lastCpuUserTime = user;
                m_lastCpuWallTimeMs = now;

                if (cpuPercent < 0.0) cpuPercent = 0.0;
                if (cpuPercent > 100.0) cpuPercent = 100.0;
                return cpuPercent;
            }
        }

        m_lastCpuKernelTime = kernel;
        m_lastCpuUserTime = user;
        m_lastCpuWallTimeMs = now;
    }
#endif
    return 0.0;
}

// ============================================================================
// ScopedPerformanceTimer
// ============================================================================

ScopedPerformanceTimer::ScopedPerformanceTimer(const QString& operationName,
                                                PerformanceMonitor* monitor)
    : m_operationName(operationName)
    , m_monitor(monitor ? monitor : PerformanceMonitor::instance())
{
    m_timer.start();
    if (m_monitor) {
        m_monitor->startOperation(m_operationName);
    }
}

ScopedPerformanceTimer::~ScopedPerformanceTimer()
{
    if (m_monitor) {
        m_monitor->endOperation(m_operationName);
    }
}

} // namespace Monitoring
} // namespace Infrastructure
