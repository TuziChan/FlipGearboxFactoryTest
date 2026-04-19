#include <QCoreApplication>
#include <QtTest>
#include <QSignalSpy>
#include <QVariantMap>

#include "src/viewmodels/RecipeViewModel.h"
#include "src/infrastructure/services/RecipeService.h"

using namespace ViewModels;
using namespace Infrastructure::Services;

class RecipeViewModelTests : public QObject {
    Q_OBJECT

private:
    RecipeService* m_service;
    RecipeViewModel* m_vm;

private slots:
    void init() {
        m_service = new RecipeService(this);
        m_vm = new RecipeViewModel(m_service, this);
    }

    void cleanup() {
        delete m_vm;
        delete m_service;
    }

    // --- Null service error handling ---

    void testNullServiceLoadAll() {
        RecipeViewModel vm(nullptr);
        QSignalSpy spy(&vm, &RecipeViewModel::lastErrorChanged);
        vm.loadAll();
        QCOMPARE(spy.count(), 1);
        QVERIFY(vm.lastError().contains("not available"));
    }

    void testNullServiceSaveEdit() {
        RecipeViewModel vm(nullptr);
        QVERIFY(!vm.saveEdit());
        QVERIFY(vm.lastError().contains("not available"));
    }

    void testNullServiceRemove() {
        RecipeViewModel vm(nullptr);
        QVERIFY(!vm.remove("test.json"));
        QVERIFY(vm.lastError().contains("not available"));
    }

    void testNullServiceExportTo() {
        RecipeViewModel vm(nullptr);
        QVERIFY(!vm.exportTo("test.json", "/tmp/test"));
    }

    void testNullServiceImportFrom() {
        RecipeViewModel vm(nullptr);
        QVariantMap result = vm.importFrom("/tmp/test.json");
        QVERIFY(result.isEmpty());
        QVERIFY(vm.lastError().contains("not available"));
    }

    // --- Dirty tracking ---

    void testBeginEditNotDirty() {
        QVariantMap recipe;
        recipe["name"] = "TestRecipe";
        recipe["fileName"] = "test.json";

        m_vm->beginEdit(recipe);
        QVERIFY(!m_vm->editingDirty());
        QCOMPARE(m_vm->editingRecipe().value("name").toString(), QString("TestRecipe"));
    }

    void testUpdateEditFieldBecomesDirty() {
        QVariantMap recipe;
        recipe["name"] = "TestRecipe";
        recipe["fileName"] = "test.json";

        m_vm->beginEdit(recipe);
        QVERIFY(!m_vm->editingDirty());

        QSignalSpy dirtySpy(m_vm, &RecipeViewModel::editingDirtyChanged);
        QSignalSpy recipeSpy(m_vm, &RecipeViewModel::editingRecipeChanged);

        m_vm->updateEditField("name", "ModifiedRecipe");
        QVERIFY(m_vm->editingDirty());
        QCOMPARE(m_vm->editingRecipe().value("name").toString(), QString("ModifiedRecipe"));
        QCOMPARE(dirtySpy.count(), 1);
        QCOMPARE(recipeSpy.count(), 1);
    }

    void testUpdateEditFieldSameValueNotDirty() {
        QVariantMap recipe;
        recipe["name"] = "TestRecipe";
        recipe["fileName"] = "test.json";

        m_vm->beginEdit(recipe);
        m_vm->updateEditField("name", "TestRecipe");
        QVERIFY(!m_vm->editingDirty());
    }

    void testCancelEditRestoresOriginal() {
        QVariantMap recipe;
        recipe["name"] = "Original";
        recipe["fileName"] = "test.json";

        m_vm->beginEdit(recipe);
        m_vm->updateEditField("name", "Modified");
        QVERIFY(m_vm->editingDirty());

        QSignalSpy dirtySpy(m_vm, &RecipeViewModel::editingDirtyChanged);
        QSignalSpy recipeSpy(m_vm, &RecipeViewModel::editingRecipeChanged);

        m_vm->cancelEdit();
        QVERIFY(!m_vm->editingDirty());
        QCOMPARE(m_vm->editingRecipe().value("name").toString(), QString("Original"));
        QCOMPARE(dirtySpy.count(), 1);
        QCOMPARE(recipeSpy.count(), 1);
    }

    void testMultipleEditsThenCancel() {
        QVariantMap recipe;
        recipe["name"] = "Original";
        recipe["fileName"] = "test.json";
        recipe["homeDutyCycle"] = 20.0;

        m_vm->beginEdit(recipe);
        m_vm->updateEditField("name", "Mod1");
        m_vm->updateEditField("homeDutyCycle", 30.0);
        m_vm->cancelEdit();

        QCOMPARE(m_vm->editingRecipe().value("name").toString(), QString("Original"));
        QCOMPARE(m_vm->editingRecipe().value("homeDutyCycle").toDouble(), 20.0);
        QVERIFY(!m_vm->editingDirty());
    }

    // --- Save edit validation ---

    void testSaveEditEmptyName() {
        QVariantMap recipe;
        recipe["name"] = "";
        recipe["fileName"] = "test.json";

        m_vm->beginEdit(recipe);
        QSignalSpy errorSpy(m_vm, &RecipeViewModel::errorOccurred);
        QVERIFY(!m_vm->saveEdit());
        QVERIFY(!m_vm->lastError().isEmpty());
        QCOMPARE(errorSpy.count(), 1);
    }

    void testSaveEditEmptyFileName() {
        QVariantMap recipe;
        recipe["name"] = "ValidName";
        recipe["fileName"] = "";

        m_vm->beginEdit(recipe);
        QVERIFY(!m_vm->saveEdit());
        QVERIFY(m_vm->lastError().contains("文件名"));
    }

    void testSaveEditDutyCycleOutOfRange() {
        QVariantMap recipe;
        recipe["name"] = "Test";
        recipe["fileName"] = "test.json";
        recipe["homeDutyCycle"] = 150.0;

        m_vm->beginEdit(recipe);
        QVERIFY(!m_vm->saveEdit());
        QVERIFY(m_vm->lastError().contains("homeDutyCycle"));
    }

    void testSaveEditNegativeDutyCycle() {
        QVariantMap recipe;
        recipe["name"] = "Test";
        recipe["fileName"] = "test.json";
        recipe["idleDutyCycle"] = -10.0;

        m_vm->beginEdit(recipe);
        QVERIFY(!m_vm->saveEdit());
        QVERIFY(m_vm->lastError().contains("idleDutyCycle"));
    }

    void testSaveEditZeroTimeoutInvalid() {
        QVariantMap recipe;
        recipe["name"] = "Test";
        recipe["fileName"] = "test.json";
        recipe["homeDutyCycle"] = 20.0;
        recipe["idleDutyCycle"] = 50.0;
        recipe["angleTestDutyCycle"] = 30.0;
        recipe["loadDutyCycle"] = 50.0;
        recipe["homeTimeoutMs"] = 0;

        m_vm->beginEdit(recipe);
        QVERIFY(!m_vm->saveEdit());
        QVERIFY(m_vm->lastError().contains("homeTimeoutMs"));
    }

    void testSaveEditValidRecipePassesValidation() {
        QVariantMap recipe;
        recipe["name"] = "ValidRecipe";
        recipe["fileName"] = "valid.json";
        recipe["homeDutyCycle"] = 20.0;
        recipe["idleDutyCycle"] = 50.0;
        recipe["angleTestDutyCycle"] = 30.0;
        recipe["loadDutyCycle"] = 50.0;
        recipe["homeTimeoutMs"] = 30000;
        recipe["angleTimeoutMs"] = 15000;

        m_vm->beginEdit(recipe);
        // saveEdit will attempt real file write, which may fail on path,
        // but validation should pass. Check that lastError does NOT contain
        // a validation error.
        m_vm->saveEdit();
        // The error (if any) should be from service, not validation
        QVERIFY(!m_vm->lastError().contains("不能为空") && !m_vm->lastError().contains("必须在"));
    }

    // --- Error propagation ---

    void testRemoveNonexistent() {
        QSignalSpy errorSpy(m_vm, &RecipeViewModel::errorOccurred);
        QVERIFY(!m_vm->remove("nonexistent.json"));
        QVERIFY(!m_vm->lastError().isEmpty());
        QCOMPARE(errorSpy.count(), 1);
    }

    void testImportFromNonexistent() {
        QVariantMap result = m_vm->importFrom("/nonexistent/path/recipe.json");
        QVERIFY(result.isEmpty());
        QVERIFY(!m_vm->lastError().isEmpty());
    }

    void testExportToNonexistentSource() {
        QVERIFY(!m_vm->exportTo("nonexistent.json", "/tmp/export.json"));
        QVERIFY(!m_vm->lastError().isEmpty());
    }

    // --- Signal emission ---

    void testBeginEditEmitsSignals() {
        QVariantMap recipe;
        recipe["name"] = "Test";

        QSignalSpy recipeSpy(m_vm, &RecipeViewModel::editingRecipeChanged);
        QSignalSpy dirtySpy(m_vm, &RecipeViewModel::editingDirtyChanged);

        m_vm->beginEdit(recipe);
        QCOMPARE(recipeSpy.count(), 1);
        QCOMPARE(dirtySpy.count(), 1);
    }

    void testLoadAllEmitsRecordsChanged() {
        QSignalSpy spy(m_vm, &RecipeViewModel::recipesChanged);
        m_vm->loadAll();
        QCOMPARE(spy.count(), 1);
    }
};

QTEST_MAIN(RecipeViewModelTests)
#include "RecipeViewModelTests.moc"
