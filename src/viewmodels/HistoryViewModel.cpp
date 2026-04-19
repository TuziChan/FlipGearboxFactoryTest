#include "HistoryViewModel.h"

namespace ViewModels {

HistoryViewModel::HistoryViewModel(Infrastructure::Services::HistoryService* service,
                                     QObject* parent)
    : QObject(parent)
    , m_service(service)
    , m_records()
    , m_filteredRecords()
    , m_lastError()
{
}

void HistoryViewModel::loadAll() {
    if (!m_service) {
        setLastError("HistoryService not available");
        return;
    }

    m_records = m_service->loadAll();
    m_filteredRecords = m_records;
    emit recordsChanged();
    emit filteredRecordsChanged();

    if (m_records.isEmpty()) {
        const QString err = m_service->lastError();
        if (!err.isEmpty()) {
            setLastError(err);
        }
    }
}

void HistoryViewModel::filter(const QString& verdictFilter,
                                const QString& dateFrom,
                                const QString& dateTo) {
    if (!m_service) {
        setLastError("HistoryService not available");
        return;
    }

    m_filteredRecords = m_service->filteredModel(verdictFilter, dateFrom, dateTo);
    emit filteredRecordsChanged();
}

bool HistoryViewModel::deleteRecord(const QString& recordId) {
    if (!m_service) {
        setLastError("HistoryService not available");
        return false;
    }

    if (!m_service->deleteRecord(recordId)) {
        setLastError(m_service->lastError());
        emit errorOccurred(m_lastError);
        return false;
    }

    loadAll();
    return true;
}

bool HistoryViewModel::exportRecord(const QString& recordId, const QString& targetPath) {
    if (!m_service) {
        setLastError("HistoryService not available");
        return false;
    }

    if (!m_service->exportRecord(recordId, targetPath)) {
        setLastError(m_service->lastError());
        emit errorOccurred(m_lastError);
        return false;
    }
    return true;
}

bool HistoryViewModel::exportAll(const QString& targetPath) {
    if (!m_service) {
        setLastError("HistoryService not available");
        return false;
    }

    if (!m_service->exportAll(targetPath)) {
        setLastError(m_service->lastError());
        emit errorOccurred(m_lastError);
        return false;
    }
    return true;
}

void HistoryViewModel::setLastError(const QString& error) {
    if (m_lastError != error) {
        m_lastError = error;
        emit lastErrorChanged();
    }
}

} // namespace ViewModels
