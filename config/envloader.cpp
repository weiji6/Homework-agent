#include "envloader.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTextStream>

QStringList EnvLoader::candidatePaths()
{
    const QString appDir = QCoreApplication::applicationDirPath();
    const QString currentDir = QDir::currentPath();

    QStringList paths;
    paths << QDir(appDir).filePath(QStringLiteral(".env"));
    paths << QDir(currentDir).filePath(QStringLiteral(".env"));
    paths << QDir(currentDir).filePath(QStringLiteral("../../.env"));
    paths << QDir(appDir).filePath(QStringLiteral("../../../.env"));
    paths.removeDuplicates();
    return paths;
}

bool EnvLoader::load(QMap<QString, QString> *values, QString *filePath, QString *errorMessage)
{
    if (!values) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("环境配置对象无效");
        }
        return false;
    }

    QString envPath;
    const QStringList paths = candidatePaths();
    for (const QString &path : paths) {
        if (QFile::exists(path)) {
            envPath = QDir::cleanPath(path);
            break;
        }
    }

    if (envPath.isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("未找到 .env 配置文件。尝试路径：\n%1")
                                .arg(paths.join(QLatin1Char('\n')));
        }
        return false;
    }

    QFile file(envPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("无法读取 .env 配置文件：%1").arg(envPath);
        }
        return false;
    }

    values->clear();
    QTextStream stream(&file);
    while (!stream.atEnd()) {
        QString key;
        QString value;
        if (parseLine(stream.readLine(), &key, &value)) {
            values->insert(key, value);
        }
    }

    if (filePath) {
        *filePath = envPath;
    }
    return true;
}

QString EnvLoader::value(const QMap<QString, QString> &values, const QString &key, const QString &fallback)
{
    const QString configValue = values.value(key).trimmed();
    if (!configValue.isEmpty()) {
        return configValue;
    }

    const QString envValue = qEnvironmentVariable(key.toUtf8().constData()).trimmed();
    return envValue.isEmpty() ? fallback : envValue;
}

bool EnvLoader::parseLine(const QString &line, QString *key, QString *value)
{
    QString trimmed = line.trimmed();
    if (trimmed.isEmpty() || trimmed.startsWith(QLatin1Char('#'))) {
        return false;
    }
    if (trimmed.startsWith(QStringLiteral("export "))) {
        trimmed = trimmed.mid(7).trimmed();
    }

    const int equalsIndex = trimmed.indexOf(QLatin1Char('='));
    if (equalsIndex <= 0) {
        return false;
    }

    const QString parsedKey = trimmed.left(equalsIndex).trimmed();
    if (parsedKey.isEmpty()) {
        return false;
    }

    *key = parsedKey;
    *value = unquote(trimmed.mid(equalsIndex + 1).trimmed());
    return true;
}

QString EnvLoader::unquote(QString value)
{
    if (value.size() >= 2) {
        const QChar first = value.front();
        const QChar last = value.back();
        if ((first == QLatin1Char('"') && last == QLatin1Char('"')) ||
            (first == QLatin1Char('\'') && last == QLatin1Char('\''))) {
            value = value.mid(1, value.size() - 2);
        }
    }

    return value.replace(QStringLiteral("\\n"), QStringLiteral("\n"));
}
