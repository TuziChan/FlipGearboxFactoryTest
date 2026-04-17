#ifndef HISTORYSERVICE_H
#define HISTORYSERVICE_H

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

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

private:
    QString dataFilePath() const;
    bool ensureDataDir() const;
    mutable QString m_lastError;
};

} // namespace Services
} // namespace Infrastructure

#endif // HISTORYSERVICE_H
