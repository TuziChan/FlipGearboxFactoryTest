#ifndef JSONREPORTWRITER_H
#define JSONREPORTWRITER_H

#include "../../domain/TestResults.h"
#include "../../domain/TelemetrySnapshot.h"
#include "../acquisition/TelemetryBuffer.h"
#include <QString>
#include <QJsonObject>
#include <QJsonArray>

namespace Infrastructure {
namespace Reporting {

class JsonReportWriter {
public:
    JsonReportWriter();

    bool writeReport(const Domain::TestResults& results,
                     const Acquisition::TelemetryBuffer& telemetry,
                     const QString& filePath);

    QString lastError() const { return m_lastError; }

private:
    QString m_lastError;

    QJsonObject serializeResults(const Domain::TestResults& results);
    QJsonObject serializeTelemetry(const Acquisition::TelemetryBuffer& telemetry);
};

} // namespace Reporting
} // namespace Infrastructure

#endif // JSONREPORTWRITER_H
