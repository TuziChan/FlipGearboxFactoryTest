#include "RecipeViewModel.h"

namespace ViewModels {

RecipeViewModel::RecipeViewModel(Infrastructure::Services::RecipeService* service,
                                   QObject* parent)
    : QObject(parent)
    , m_service(service)
    , m_recipes()
    , m_editingRecipe()
    , m_originalRecipe()
    , m_editingDirty(false)
    , m_lastError()
{
}

void RecipeViewModel::loadAll() {
    if (!m_service) {
        setLastError("RecipeService not available");
        return;
    }

    m_recipes = m_service->loadAll();
    emit recipesChanged();

    if (m_recipes.isEmpty()) {
        const QString err = m_service->lastError();
        if (!err.isEmpty()) {
            setLastError(err);
        }
    }
}

void RecipeViewModel::beginEdit(const QVariantMap& recipeData) {
    m_editingRecipe = recipeData;
    m_originalRecipe = recipeData;
    m_editingDirty = false;
    emit editingRecipeChanged();
    emit editingDirtyChanged();
}

void RecipeViewModel::updateEditField(const QString& field, const QVariant& value) {
    if (m_editingRecipe.value(field) == value) {
        return;
    }

    m_editingRecipe[field] = value;
    m_editingDirty = m_editingRecipe != m_originalRecipe;
    emit editingRecipeChanged();
    emit editingDirtyChanged();
}

bool RecipeViewModel::saveEdit() {
    if (!m_service) {
        setLastError("RecipeService not available");
        return false;
    }

    if (!validateEditingRecipe()) {
        return false;
    }

    if (!m_service->save(m_editingRecipe)) {
        setLastError(m_service->lastError());
        emit errorOccurred(m_lastError);
        return false;
    }

    m_originalRecipe = m_editingRecipe;
    m_editingDirty = false;
    emit editingDirtyChanged();

    return true;
}

void RecipeViewModel::cancelEdit() {
    m_editingRecipe = m_originalRecipe;
    m_editingDirty = false;
    emit editingRecipeChanged();
    emit editingDirtyChanged();
}

bool RecipeViewModel::remove(const QString& fileName) {
    if (!m_service) {
        setLastError("RecipeService not available");
        return false;
    }

    if (!m_service->remove(fileName)) {
        setLastError(m_service->lastError());
        emit errorOccurred(m_lastError);
        return false;
    }

    loadAll();
    return true;
}

bool RecipeViewModel::exportTo(const QString& fileName, const QString& targetPath) {
    if (!m_service) {
        setLastError("RecipeService not available");
        return false;
    }

    if (!m_service->exportTo(fileName, targetPath)) {
        setLastError(m_service->lastError());
        emit errorOccurred(m_lastError);
        return false;
    }
    return true;
}

QVariantMap RecipeViewModel::importFrom(const QString& sourcePath) {
    if (!m_service) {
        setLastError("RecipeService not available");
        return {};
    }

    QVariantMap result = m_service->importFrom(sourcePath);
    if (result.isEmpty()) {
        setLastError(m_service->lastError());
        emit errorOccurred(m_lastError);
        return {};
    }

    loadAll();
    return result;
}

void RecipeViewModel::setLastError(const QString& error) {
    if (m_lastError != error) {
        m_lastError = error;
        emit lastErrorChanged();
    }
}

bool RecipeViewModel::validateEditingRecipe() {
    const QString name = m_editingRecipe.value("name").toString().trimmed();
    if (name.isEmpty()) {
        setLastError("配方名称不能为空");
        emit errorOccurred(m_lastError);
        return false;
    }

    const QString fileName = m_editingRecipe.value("fileName").toString().trimmed();
    if (fileName.isEmpty()) {
        setLastError("文件名不能为空");
        emit errorOccurred(m_lastError);
        return false;
    }

    auto checkRange = [&](const QString& field, double min, double max) -> bool {
        bool ok = false;
        const double val = m_editingRecipe.value(field).toDouble(&ok);
        if (!ok || val < min || val > max) {
            setLastError(QString("字段 %1 的值必须在 %2 ~ %3 之间").arg(field).arg(min).arg(max));
            emit errorOccurred(m_lastError);
            return false;
        }
        return true;
    };

    auto checkPositive = [&](const QString& field) -> bool {
        return checkRange(field, 0.001, 1e9);
    };

    if (!checkRange("homeDutyCycle", 0.0, 100.0)) return false;
    if (!checkRange("idleDutyCycle", 0.0, 100.0)) return false;
    if (!checkRange("angleTestDutyCycle", 0.0, 100.0)) return false;
    if (!checkRange("loadDutyCycle", 0.0, 100.0)) return false;
    if (!checkRange("impactDutyCycle", 0.0, 100.0)) return false;
    if (!checkPositive("homeTimeoutMs")) return false;
    if (!checkPositive("angleTimeoutMs")) return false;

    // Impact test validation (only when enabled)
    if (m_editingRecipe.value("impactTestEnabled").toBool()) {
        if (!checkPositive("impactSpinupMs")) return false;
        if (!checkRange("impactCycles", 1.0, 100.0)) return false;
        if (!checkPositive("impactBrakeCurrentA")) return false;
        if (!checkPositive("impactBrakeOnMs")) return false;
        if (!checkPositive("impactBrakeOffMs")) return false;
    }

    return true;
}

} // namespace ViewModels
