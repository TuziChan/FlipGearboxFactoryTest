#ifndef HISTORYVIEWMODEL_H
#define HISTORYVIEWMODEL_H

#include <QObject>
#include <QVariantList>
#include <QVariantMap>
#include "../infrastructure/services/HistoryService.h"

namespace ViewModels {

class HistoryViewModel : public QObject {
    Q_OBJECT

    Q_PROPERTY(QVariantList records READ records NOTIFY recordsChanged)
    Q_PROPERTY(QVariantList filteredRecords READ filteredRecords NOTIFY filteredRecordsChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    explicit HistoryViewModel(Infrastructure::Services::HistoryService* service,
                               QObject* parent = nullptr);

    QVariantList records() const { return m_records; }
    QVariantList filteredRecords() const { return m_filteredRecords; }
    QString lastError() const { return m_lastError; }

    Q_INVOKABLE void loadAll();
    Q_INVOKABLE void filter(const QString& verdictFilter,
                             const QString& dateFrom,
                             const QString& dateTo);
    Q_INVOKABLE bool deleteRecord(const QString& recordId);
    Q_INVOKABLE bool exportRecord(const QString& recordId, const QString& targetPath);
    Q_INVOKABLE bool exportAll(const QString& targetPath);

signals:
    void recordsChanged();
    void filteredRecordsChanged();
    void lastErrorChanged();
    void errorOccurred(const QString& message);

private:
    Infrastructure::Services::HistoryService* m_service;
    QVariantList m_records;
    QVariantList m_filteredRecords;
    QString m_lastError;

    void setLastError(const QString& error);
};

} // namespace ViewModels

#endif // HISTORYVIEWMODEL_H
