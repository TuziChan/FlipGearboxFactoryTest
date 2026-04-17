#include "RecipeService.h"
#include "../config/RecipeConfig.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>

namespace Infrastructure {
namespace Services {

RecipeService::RecipeService(QObject* parent)
    : QObject(parent)
{}

QString RecipeService::recipesPath() const {
    QDir appDir(QCoreApplication::applicationDirPath());
    appDir.cdUp();
    appDir.cdUp();
    return appDir.filePath("config/recipes");
}

bool RecipeService::setError(const QString& error) const {
    m_lastError = error;
    return false;
}

QVariantList RecipeService::loadAll() {
    QVariantList result;
    const QString dirPath = recipesPath();
    QDir dir(dirPath);

    if (!dir.exists()) {
        setError(QString("Recipe directory does not exist: %1").arg(dirPath));
        return result;
    }

    const QStringList filters{"*.json"};
    const QFileInfoList entries = dir.entryInfoList(filters, QDir::Files, QDir::Name);

    for (const QFileInfo& fi : entries) {
        QFile file(fi.absoluteFilePath());
        if (!file.open(QIODevice::ReadOnly)) {
            continue;
        }

        const QByteArray data = file.readAll();
        file.close();

        QJsonParseError parseError;
        const QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
        if (doc.isNull() || !doc.isObject()) {
            continue;
        }

        const Domain::TestRecipe recipe = Config::RecipeConfig::fromJson(doc.object());
        QJsonObject jsonObj = Config::RecipeConfig::toJson(recipe);
        QVariantMap map = jsonObj.toVariantMap();
        map["fileName"] = fi.fileName();
        map["filePath"] = fi.absoluteFilePath();

        result.append(map);
    }

    m_lastError.clear();
    return result;
}

bool RecipeService::save(const QVariantMap& recipeData) {
    const QString fileName = recipeData.value("fileName").toString();
    if (fileName.isEmpty()) {
        return setError("fileName is empty");
    }

    const QString dirPath = recipesPath();
    QDir dir(dirPath);
    if (!dir.exists() && !dir.mkpath(".")) {
        return setError(QString("Failed to create recipe directory: %1").arg(dirPath));
    }

    QVariantMap cleanData = recipeData;
    cleanData.remove("fileName");
    cleanData.remove("filePath");

    const QJsonObject jsonObj = QJsonObject::fromVariantMap(cleanData);
    const QByteArray data = QJsonDocument(jsonObj).toJson(QJsonDocument::Indented);

    const QString filePath = dir.filePath(fileName);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return setError(QString("Failed to open file for writing: %1").arg(filePath));
    }

    file.write(data);
    file.close();

    m_lastError.clear();
    return true;
}

bool RecipeService::remove(const QString& fileName) {
    if (fileName.isEmpty()) {
        return setError("fileName is empty");
    }

    const QString filePath = QDir(recipesPath()).filePath(fileName);
    QFile file(filePath);
    if (!file.exists()) {
        return setError(QString("File does not exist: %1").arg(filePath));
    }

    if (!file.remove()) {
        return setError(QString("Failed to delete file: %1").arg(filePath));
    }

    m_lastError.clear();
    return true;
}

bool RecipeService::exportTo(const QString& fileName, const QString& targetPath) {
    if (fileName.isEmpty()) {
        return setError("fileName is empty");
    }

    const QString srcPath = QDir(recipesPath()).filePath(fileName);
    QFile srcFile(srcPath);
    if (!srcFile.exists()) {
        return setError(QString("Source file does not exist: %1").arg(srcPath));
    }

    if (!srcFile.open(QIODevice::ReadOnly)) {
        return setError(QString("Failed to open source file: %1").arg(srcPath));
    }

    const QByteArray data = srcFile.readAll();
    srcFile.close();

    QFileInfo targetInfo(targetPath);
    QDir targetDir = targetInfo.absoluteDir();
    if (!targetDir.exists() && !targetDir.mkpath(".")) {
        return setError(QString("Failed to create target directory: %1").arg(targetDir.path()));
    }

    QFile dstFile(targetPath);
    if (!dstFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return setError(QString("Failed to open target file for writing: %1").arg(targetPath));
    }

    dstFile.write(data);
    dstFile.close();

    m_lastError.clear();
    return true;
}

QVariantMap RecipeService::importFrom(const QString& sourcePath) {
    QFile file(sourcePath);
    if (!file.exists()) {
        setError(QString("Source file does not exist: %1").arg(sourcePath));
        return {};
    }

    if (!file.open(QIODevice::ReadOnly)) {
        setError(QString("Failed to open source file: %1").arg(sourcePath));
        return {};
    }

    const QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (doc.isNull() || !doc.isObject()) {
        setError(QString("Invalid JSON in file: %1").arg(sourcePath));
        return {};
    }

    const Domain::TestRecipe recipe = Config::RecipeConfig::fromJson(doc.object());
    QJsonObject jsonObj = Config::RecipeConfig::toJson(recipe);

    const QString fileName = QFileInfo(sourcePath).fileName();
    const QString destPath = QDir(recipesPath()).filePath(fileName);

    QDir destDir(recipesPath());
    if (!destDir.exists() && !destDir.mkpath(".")) {
        setError(QString("Failed to create recipe directory: %1").arg(destDir.path()));
        return {};
    }

    QFile destFile(destPath);
    if (!destFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        setError(QString("Failed to write imported recipe: %1").arg(destPath));
        return {};
    }

    destFile.write(QJsonDocument(jsonObj).toJson(QJsonDocument::Indented));
    destFile.close();

    QVariantMap map = jsonObj.toVariantMap();
    map["fileName"] = fileName;
    map["filePath"] = destPath;

    m_lastError.clear();
    return map;
}

QString RecipeService::lastError() const {
    return m_lastError;
}

} // namespace Services
} // namespace Infrastructure
