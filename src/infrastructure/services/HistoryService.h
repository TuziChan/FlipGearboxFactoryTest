#ifndef HISTORYSERVICE_H
#define HISTORYSERVICE_H

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>
#include <QReadWriteLock>
#include <QMutex>

namespace Infrastructure {
namespace Services {

class HistoryService : public QObject {
Q_OBJECT
public:
explicit HistoryService(QObject* parent = nullptr);

Q_INVOKABLE QVariantList loadAll();
Q_INVOKABLE bool addRecord(const QVariantMap& record);
Q_INVOKABLE bool exportRecord(const QString& recordId, const QString& targetPath);
Q_INVOKABLE bool exportAll(const QString& targetPath);
Q_INVOKABLE bool deleteRecord(const QString& recordId);
Q_INVOKABLE QVariantList filteredModel(const QString& verdictFilter,
const QString& dateFrom,
const QString& dateTo) const;
Q_INVOKABLE QString lastError() const;

// Cache management
Q_INVOKABLE void invalidateCache();

private:
QString dataFilePath() const;
bool ensureDataDir() const;
mutable QString m_lastError;

// Thread-safe cache with read-write lock
mutable QVariantList m_cachedRecords;
mutable bool m_cacheValid;
mutable QReadWriteLock m_cacheLock;   // Protects m_cachedRecords and m_cacheValid
mutable QMutex m_fileLock;           // Protects file I/O operations

// Atomic file write helper
bool writeAtomically(const QString& path, const QByteArray& data);
};

} // namespace Services
} // namespace Infrastructure

#endif // HISTORYSERVICE_H
