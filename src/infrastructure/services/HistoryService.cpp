#include "HistoryService.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace Infrastructure {
namespace Services {

HistoryService::HistoryService(QObject* parent)
    : QObject(parent)
    , m_lastError()
    , m_cacheValid(false)
{
}

QString HistoryService::dataFilePath() const
{
    QDir appDir(QCoreApplication::applicationDirPath());
    appDir.cdUp();
    appDir.cdUp();
    return appDir.filePath(QStringLiteral("data/history.jsonl"));
}

bool HistoryService::ensureDataDir() const
{
    QDir appDir(QCoreApplication::applicationDirPath());
    appDir.cdUp();
    appDir.cdUp();
    if (!appDir.exists(QStringLiteral("data"))) {
        if (!appDir.mkpath(QStringLiteral("data"))) {
            m_lastError = tr("Failed to create data directory: %1")
                              .arg(appDir.filePath(QStringLiteral("data")));
            return false;
        }
    }
    return true;
}

QVariantList HistoryService::loadAll()
{
    // Check if cache is valid
    if (m_cacheValid) {
        return m_cachedRecords;
    }

    QVariantList records;
    const QString filePath = dataFilePath();
    QFile file(filePath);

    if (!file.exists()) {
        m_cachedRecords = records;
        m_cacheValid = true;
        return records;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_lastError = tr("Failed to open history file: %1").arg(file.errorString());
        return records;
    }

    while (!file.atEnd()) {
        const QByteArray line = file.readLine().trimmed();
        if (line.isEmpty()) {
            continue;
        }
        QJsonParseError parseError;
        const QJsonDocument doc = QJsonDocument::fromJson(line, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            continue;
        }
        if (doc.isObject()) {
            records.append(doc.object().toVariantMap());
        }
    }
    file.close();

    // Sort by startTime descending (most recent first)
    std::sort(records.begin(), records.end(),
              [](const QVariant& a, const QVariant& b) {
                  const QString timeA = a.toMap().value(QStringLiteral("startTime")).toString();
                  const QString timeB = b.toMap().value(QStringLiteral("startTime")).toString();
                  return timeA > timeB;
              });

    // Update cache
    m_cachedRecords = records;
    m_cacheValid = true;

    return records;
}

bool HistoryService::addRecord(const QVariantMap& record)
{
    if (!ensureDataDir()) {
        return false;
    }

    const QString filePath = dataFilePath();
    QFile file(filePath);

    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        m_lastError = tr("Failed to open history file for writing: %1").arg(file.errorString());
        return false;
    }

    const QJsonObject jsonObj = QJsonObject::fromVariantMap(record);
    const QJsonDocument doc(jsonObj);
    file.write(doc.toJson(QJsonDocument::Compact));
    file.write("\n");
    file.close();

    // Invalidate cache
    m_cacheValid = false;

    return true;
}

bool HistoryService::exportRecord(const QString& recordId, const QString& targetPath)
{
    const QVariantList all = loadAll();
    for (const QVariant& item : all) {
        const QVariantMap record = item.toMap();
        if (record.value(QStringLiteral("id")).toString() == recordId) {
            QFile file(targetPath);
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                m_lastError = tr("Failed to open export file: %1").arg(file.errorString());
                return false;
            }
            const QJsonObject jsonObj = QJsonObject::fromVariantMap(record);
            const QJsonDocument doc(jsonObj);
            file.write(doc.toJson(QJsonDocument::Indented));
            file.close();
            return true;
        }
    }

    m_lastError = tr("Record not found: %1").arg(recordId);
    return false;
}

bool HistoryService::exportAll(const QString& targetPath)
{
    const QVariantList records = loadAll();
    QJsonArray array;
    for (const QVariant& item : records) {
        const QJsonObject obj = QJsonObject::fromVariantMap(item.toMap());
        array.append(obj);
    }

    QFile file(targetPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = tr("Failed to open export file: %1").arg(file.errorString());
        return false;
    }

    const QJsonDocument doc(array);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

bool HistoryService::deleteRecord(const QString& recordId)
{
    QVariantList all = loadAll();
    QVariantList filtered;
    bool found = false;
    for (const QVariant& item : all) {
        const QVariantMap record = item.toMap();
        if (record.value(QStringLiteral("id")).toString() == recordId) {
            found = true;
            continue;
        }
        filtered.append(item);
    }

    if (!found) {
        m_lastError = tr("Record not found: %1").arg(recordId);
        return false;
    }

    ensureDataDir();
    QFile file(dataFilePath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        m_lastError = tr("Failed to open data file for writing: %1").arg(file.errorString());
        return false;
    }

    for (const QVariant& item : filtered) {
        const QJsonObject obj = QJsonObject::fromVariantMap(item.toMap());
        const QJsonDocument doc(obj);
        file.write(doc.toJson(QJsonDocument::Compact));
        file.write("\n");
    }
    file.close();

    // Invalidate cache
    m_cacheValid = false;

    return true;
}

QVariantList HistoryService::filteredModel(const QString& verdictFilter,
                                           const QString& dateFrom,
                                           const QString& dateTo) const
{
    // Load all records (uses cache if available)
    QVariantList all;
    {
        HistoryService* nonConst = const_cast<HistoryService*>(this);
        all = nonConst->loadAll();
    }

    // Early return if no filters applied
    const bool hasVerdictFilter = (verdictFilter != QStringLiteral("ALL"));
    const QDate fromDate = QDate::fromString(dateFrom, QStringLiteral("yyyy-MM-dd"));
    const QDate toDate = QDate::fromString(dateTo, QStringLiteral("yyyy-MM-dd"));
    const bool hasDateFilter = fromDate.isValid() || toDate.isValid();

    if (!hasVerdictFilter && !hasDateFilter) {
        return all;
    }

    // Apply filters efficiently
    QVariantList filtered;
    filtered.reserve(all.size()); // Pre-allocate to avoid reallocations

    for (const QVariant& item : all) {
        const QVariantMap record = item.toMap();

        // Verdict filter
        if (hasVerdictFilter) {
            const QString verdict = record.value(QStringLiteral("verdict")).toString();
            if (verdict != verdictFilter) {
                continue;
            }
        }

        // Date filter
        if (hasDateFilter) {
            const QString startTimeStr = record.value(QStringLiteral("startTime")).toString();
            const QDateTime startDt = QDateTime::fromString(startTimeStr, Qt::ISODate);
            const QDate startDate = startDt.date();

            if (fromDate.isValid() && startDate < fromDate) {
                continue;
            }

            if (toDate.isValid() && startDate > toDate) {
                continue;
            }
        }

        filtered.append(record);
    }

    return filtered;
}

void HistoryService::invalidateCache()
{
    m_cacheValid = false;
    m_cachedRecords.clear();
}

QString HistoryService::lastError() const
{
    return m_lastError;
}

} // namespace Services
} // namespace Infrastructure
