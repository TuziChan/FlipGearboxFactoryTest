#ifndef RECIPEVIEWMODEL_H
#define RECIPEVIEWMODEL_H

#include <QObject>
#include <QVariantList>
#include <QVariantMap>
#include "../infrastructure/services/RecipeService.h"

namespace ViewModels {

class RecipeViewModel : public QObject {
    Q_OBJECT

    Q_PROPERTY(QVariantList recipes READ recipes NOTIFY recipesChanged)
    Q_PROPERTY(QVariantMap editingRecipe READ editingRecipe NOTIFY editingRecipeChanged)
    Q_PROPERTY(bool editingDirty READ editingDirty NOTIFY editingDirtyChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    explicit RecipeViewModel(Infrastructure::Services::RecipeService* service,
                               QObject* parent = nullptr);

    QVariantList recipes() const { return m_recipes; }
    QVariantMap editingRecipe() const { return m_editingRecipe; }
    bool editingDirty() const { return m_editingDirty; }
    QString lastError() const { return m_lastError; }

    Q_INVOKABLE void loadAll();
    Q_INVOKABLE void beginEdit(const QVariantMap& recipeData);
    Q_INVOKABLE void updateEditField(const QString& field, const QVariant& value);
    Q_INVOKABLE bool saveEdit();
    Q_INVOKABLE void cancelEdit();
    Q_INVOKABLE bool remove(const QString& fileName);
    Q_INVOKABLE bool exportTo(const QString& fileName, const QString& targetPath);
    Q_INVOKABLE QVariantMap importFrom(const QString& sourcePath);

signals:
    void recipesChanged();
    void editingRecipeChanged();
    void editingDirtyChanged();
    void lastErrorChanged();
    void errorOccurred(const QString& message);

private:
    Infrastructure::Services::RecipeService* m_service;
    QVariantList m_recipes;
    QVariantMap m_editingRecipe;
    QVariantMap m_originalRecipe;
    bool m_editingDirty;
    QString m_lastError;

    void setLastError(const QString& error);
    bool validateEditingRecipe();
};

} // namespace ViewModels

#endif // RECIPEVIEWMODEL_H
