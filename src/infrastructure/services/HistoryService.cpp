#include "HistoryService.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QSaveFile>
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

bool HistoryService::writeAtomically(const QString& path, const QByteArray& data) {
QSaveFile saveFile(path);
if (!saveFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
m_lastError = tr("Failed to open file for writing: %1").arg(saveFile.errorString());
return false;
}

if (saveFile.write(data) == -1) {
m_lastError = tr("Failed to write data: %1").arg(saveFile.errorString());
saveFile.cancelWriting();
return false;
}

if (!saveFile.commit()) {
m_lastError = tr("Failed to commit write: %1").arg(saveFile.errorString());
return false;
}

return true;
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
// Check if cache is valid (use read lock for thread-safe reading)
{
QReadLocker locker(&m_cacheLock);
if (m_cacheValid) {
return m_cachedRecords;
}
}

QVariantList records;
const QString filePath = dataFilePath();

// Use mutex to protect file I/O
{
QMutexLocker locker(&m_fileLock);
QFile file(filePath);

if (!file.exists()) {
QWriteLocker cacheLocker(&m_cacheLock);
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
}

// Sort by startTime descending (most recent first)
std::sort(records.begin(), records.end(),
[](const QVariant& a, const QVariant& b) {
const QString timeA = a.toMap().value(QStringLiteral("startTime")).toString();
const QString timeB = b.toMap().value(QStringLiteral("startTime")).toString();
return timeA > timeB;
});

// Update cache (use write lock)
{
QWriteLocker cacheLocker(&m_cacheLock);
m_cachedRecords = records;
m_cacheValid = true;
}

return records;
}

bool HistoryService::addRecord(const QVariantMap& record)
{
if (!ensureDataDir()) {
return false;
}

// Use mutex to protect file I/O
QMutexLocker locker(&m_fileLock);
const QString filePath = dataFilePath();

// Read existing records
QByteArray allData;
QFile existingFile(filePath);
if (existingFile.exists()) {
if (!existingFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
m_lastError = tr("Failed to read existing history file: %1").arg(existingFile.errorString());
return false;
}
allData = existingFile.readAll();
existingFile.close();
}

// Append new record
const QJsonObject jsonObj = QJsonObject::fromVariantMap(record);
const QJsonDocument doc(jsonObj);
allData.append(doc.toJson(QJsonDocument::Compact));
allData.append("\n");

// Write atomically
if (!writeAtomically(filePath, allData)) {
return false;
}

// Invalidate cache (use write lock)
{
QWriteLocker cacheLocker(&m_cacheLock);
m_cacheValid = false;
}

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

// Build new file content
QByteArray newData;
for (const QVariant& item : filtered) {
const QJsonObject obj = QJsonObject::fromVariantMap(item.toMap());
const QJsonDocument doc(obj);
newData.append(doc.toJson(QJsonDocument::Compact));
newData.append("\n");
}

// Use mutex to protect file I/O and write atomically
QMutexLocker locker(&m_fileLock);
const QString filePath = dataFilePath();

if (!writeAtomically(filePath, newData)) {
return false;
}

// Invalidate cache (use write lock)
{
QWriteLocker cacheLocker(&m_cacheLock);
m_cacheValid = false;
}

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
    QWriteLocker locker(&m_cacheLock);
    m_cacheValid = false;
    m_cachedRecords.clear();
}

QString HistoryService::lastError() const
{
    return m_lastError;
}

} // namespace Services
} // namespace Infrastructure
