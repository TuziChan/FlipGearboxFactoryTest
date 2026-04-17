#ifndef RECIPESERVICE_H
#define RECIPESERVICE_H

#include <QObject>
#include <QVariantList>
#include <QVariantMap>
#include <QString>

namespace Infrastructure {
namespace Services {

class RecipeService : public QObject {
    Q_OBJECT

public:
    explicit RecipeService(QObject* parent = nullptr);

    Q_INVOKABLE QVariantList loadAll();
    Q_INVOKABLE bool save(const QVariantMap& recipeData);
    Q_INVOKABLE bool remove(const QString& fileName);
    Q_INVOKABLE bool exportTo(const QString& fileName, const QString& targetPath);
    Q_INVOKABLE QVariantMap importFrom(const QString& sourcePath);
    Q_INVOKABLE QString lastError() const;

private:
    QString recipesPath() const;
    bool setError(const QString& error) const;

    mutable QString m_lastError;
};

} // namespace Services
} // namespace Infrastructure

#endif // RECIPESERVICE_H
