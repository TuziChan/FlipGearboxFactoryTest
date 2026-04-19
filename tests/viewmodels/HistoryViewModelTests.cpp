#include <QCoreApplication>
#include <QtTest>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

#include "src/viewmodels/HistoryViewModel.h"
#include "src/infrastructure/services/HistoryService.h"

using namespace ViewModels;
using namespace Infrastructure::Services;

class HistoryViewModelTests : public QObject {
    Q_OBJECT

private:
    HistoryService* m_service;
    HistoryViewModel* m_vm;
    QTemporaryDir m_tempDir;

    void createTestRecord(const QString& id, const QString& verdict, const QString& startTime) {
        QVariantMap record;
        record["id"] = id;
        record["verdict"] = verdict;
        record["startTime"] = startTime;
        record["serialNumber"] = "SN" + id;
        m_service->addRecord(record);
    }

private slots:
    void initTestCase() {
        QVERIFY(m_tempDir.isValid());
    }

    void init() {
        m_service = new HistoryService(this);
        m_vm = new HistoryViewModel(m_service, this);
    }

    void cleanup() {
        delete m_vm;
        delete m_service;
    }

    void testNullServiceLoadAll() {
        HistoryViewModel vm(nullptr);
        QSignalSpy spy(&vm, &HistoryViewModel::lastErrorChanged);
        vm.loadAll();
        QCOMPARE(spy.count(), 1);
        QVERIFY(vm.lastError().contains("not available"));
        QVERIFY(vm.records().isEmpty());
    }

    void testNullServiceFilter() {
        HistoryViewModel vm(nullptr);
        QSignalSpy spy(&vm, &HistoryViewModel::lastErrorChanged);
        vm.filter("ALL", "", "");
        QCOMPARE(spy.count(), 1);
    }

    void testNullServiceDeleteRecord() {
        HistoryViewModel vm(nullptr);
        QVERIFY(!vm.deleteRecord("nonexistent"));
        QVERIFY(vm.lastError().contains("not available"));
    }

    void testLoadAllEmpty() {
        QSignalSpy spy(m_vm, &HistoryViewModel::recordsChanged);
        m_vm->loadAll();
        QCOMPARE(spy.count(), 1);
        QVERIFY(m_vm->records().isEmpty());
        QVERIFY(m_vm->filteredRecords().isEmpty());
    }

    void testLoadAllWithRecords() {
        createTestRecord("r1", "PASSED", "2026-01-15T10:00:00");
        createTestRecord("r2", "FAILED", "2026-01-16T10:00:00");

        m_vm->loadAll();
        QCOMPARE(m_vm->records().size(), 2);
        QCOMPARE(m_vm->filteredRecords().size(), 2);
    }

    void testFilterByVerdict() {
        createTestRecord("r1", "PASSED", "2026-01-15T10:00:00");
        createTestRecord("r2", "FAILED", "2026-01-16T10:00:00");
        createTestRecord("r3", "PASSED", "2026-01-17T10:00:00");

        m_vm->loadAll();
        QCOMPARE(m_vm->records().size(), 3);

        m_vm->filter("PASSED", "", "");
        QCOMPARE(m_vm->filteredRecords().size(), 2);
        for (const auto& item : m_vm->filteredRecords()) {
            QCOMPARE(item.toMap().value("verdict").toString(), QString("PASSED"));
        }
    }

    void testFilterByDateRange() {
        createTestRecord("r1", "PASSED", "2026-01-10T10:00:00");
        createTestRecord("r2", "PASSED", "2026-01-15T10:00:00");
        createTestRecord("r3", "FAILED", "2026-01-20T10:00:00");

        m_vm->loadAll();
        m_vm->filter("ALL", "2026-01-12", "2026-01-18");
        QCOMPARE(m_vm->filteredRecords().size(), 1);
        QCOMPARE(m_vm->filteredRecords().first().toMap().value("id").toString(), QString("r2"));
    }

    void testFilterAll() {
        createTestRecord("r1", "PASSED", "2026-01-15T10:00:00");
        createTestRecord("r2", "FAILED", "2026-01-16T10:00:00");

        m_vm->loadAll();
        m_vm->filter("ALL", "", "");
        QCOMPARE(m_vm->filteredRecords().size(), 2);
    }

    void testDeleteRecordSuccess() {
        createTestRecord("r1", "PASSED", "2026-01-15T10:00:00");
        m_vm->loadAll();
        QCOMPARE(m_vm->records().size(), 1);

        QVERIFY(m_vm->deleteRecord("r1"));
        QCOMPARE(m_vm->records().size(), 0);
        QVERIFY(m_vm->lastError().isEmpty());
    }

    void testDeleteRecordNotFound() {
        m_vm->loadAll();
        QSignalSpy errorSpy(m_vm, &HistoryViewModel::errorOccurred);
        QVERIFY(!m_vm->deleteRecord("nonexistent"));
        QVERIFY(!m_vm->lastError().isEmpty());
        QCOMPARE(errorSpy.count(), 1);
    }

    void testExportRecordNotFound() {
        m_vm->loadAll();
        QVERIFY(!m_vm->exportRecord("nonexistent", "/tmp/test_export.json"));
        QVERIFY(!m_vm->lastError().isEmpty());
    }

    void testExportRecordSuccess() {
        createTestRecord("r1", "PASSED", "2026-01-15T10:00:00");
        m_vm->loadAll();

        const QString exportPath = m_tempDir.path() + "/export_test.json";
        QVERIFY(m_vm->exportRecord("r1", exportPath));
        QVERIFY(QFile::exists(exportPath));
    }

    void testExportAllSuccess() {
        createTestRecord("r1", "PASSED", "2026-01-15T10:00:00");
        createTestRecord("r2", "FAILED", "2026-01-16T10:00:00");
        m_vm->loadAll();

        const QString exportPath = m_tempDir.path() + "/export_all_test.json";
        QVERIFY(m_vm->exportAll(exportPath));
        QVERIFY(QFile::exists(exportPath));
    }

    void testExportAllNullService() {
        HistoryViewModel vm(nullptr);
        QVERIFY(!vm.exportAll("/tmp/test.json"));
        QVERIFY(vm.lastError().contains("not available"));
    }

    void testExportRecordNullService() {
        HistoryViewModel vm(nullptr);
        QVERIFY(!vm.exportRecord("r1", "/tmp/test.json"));
    }

    void testNullServiceExportAll() {
        HistoryViewModel vm(nullptr);
        QVERIFY(!vm.exportAll("/tmp/test.json"));
    }
};

QTEST_MAIN(HistoryViewModelTests)
#include "HistoryViewModelTests.moc"
