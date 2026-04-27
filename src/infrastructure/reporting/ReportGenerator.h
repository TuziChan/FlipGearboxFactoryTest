#ifndef REPORTGENERATOR_H
#define REPORTGENERATOR_H

#include "../../domain/TestResults.h"
#include "../acquisition/TelemetryBuffer.h"
#include <QObject>
#include <QString>

namespace Infrastructure {
namespace Reporting {

class ReportGenerator : public QObject {
    Q_OBJECT

public:
    explicit ReportGenerator(const QString& reportDir = "reports", QObject* parent = nullptr);

    bool generateReport(const Domain::TestResults& results,
                        const Acquisition::TelemetryBuffer& telemetry);

signals:
    void reportGenerated(const QString& filePath);

private:
    QString m_reportDir;

    QString generateFileName(const QString& serialNumber, const QDateTime& timestamp) const;
};

} // namespace Reporting
} // namespace Infrastructure

#endif // REPORTGENERATOR_H
