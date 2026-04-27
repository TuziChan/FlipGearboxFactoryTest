#include "ReportGenerator.h"
#include "JsonReportWriter.h"
#include <QDir>
#include <QDebug>

namespace Infrastructure {
namespace Reporting {

ReportGenerator::ReportGenerator(const QString& reportDir, QObject* parent)
    : QObject(parent)
    , m_reportDir(reportDir)
{
    QDir dir;
    if (!dir.exists(m_reportDir)) {
        dir.mkpath(m_reportDir);
    }
}

bool ReportGenerator::generateReport(const Domain::TestResults& results,
                                      const Acquisition::TelemetryBuffer& telemetry) {
    QString fileName = generateFileName(results.serialNumber, results.startTime);
    QString filePath = QDir(m_reportDir).absoluteFilePath(fileName);

    JsonReportWriter writer;
    if (!writer.writeReport(results, telemetry, filePath)) {
        qCritical() << "Failed to write report:" << writer.lastError();
        return false;
    }

    qInfo() << "Report generated successfully:" << filePath;
    emit reportGenerated(filePath);
    return true;
}

QString ReportGenerator::generateFileName(const QString& serialNumber, const QDateTime& timestamp) const {
    QString dateTime = timestamp.toString("yyyyMMdd_HHmmss");
    return QString("%1_%2.json").arg(serialNumber, dateTime);
}

} // namespace Reporting
} // namespace Infrastructure
