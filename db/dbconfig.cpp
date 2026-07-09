#include "dbconfig.h"

#include "config/envloader.h"

#include <QMap>

QStringList DbConfig::candidatePaths()
{
    return EnvLoader::candidatePaths();
}

bool DbConfig::load(DbConfig *config, QString *errorMessage)
{
    if (!config) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("数据库配置对象无效");
        }
        return false;
    }

    QMap<QString, QString> values;
    QString filePath;
    if (!EnvLoader::load(&values, &filePath, errorMessage)) {
        return false;
    }

    config->host = EnvLoader::value(values, QStringLiteral("DB_HOST"), config->host).trimmed();
    config->port = EnvLoader::value(values, QStringLiteral("DB_PORT"), QString::number(config->port)).toInt();
    config->databaseName = EnvLoader::value(values, QStringLiteral("DB_NAME"), config->databaseName).trimmed();
    config->username = EnvLoader::value(values, QStringLiteral("DB_USER"), config->username).trimmed();
    config->password = EnvLoader::value(values, QStringLiteral("DB_PASSWORD"), config->password);
    config->filePath = filePath;

    if (config->host.isEmpty() || config->databaseName.isEmpty() || config->username.isEmpty() || config->port <= 0) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("数据库配置不完整，请检查 %1 中的 DB_HOST、DB_PORT、DB_NAME、DB_USER。")
                                .arg(filePath);
        }
        return false;
    }

    return true;
}
