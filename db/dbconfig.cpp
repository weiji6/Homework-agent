#include "dbconfig.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QSettings>

QStringList DbConfig::candidatePaths()
{
    const QString appDir = QCoreApplication::applicationDirPath();
    const QString currentDir = QDir::currentPath();

    QStringList paths;
    paths << QDir(appDir).filePath(QStringLiteral("config/database.ini"));
    paths << QDir(currentDir).filePath(QStringLiteral("config/database.ini"));
    paths << QDir(currentDir).filePath(QStringLiteral("../../config/database.ini"));
    paths << QDir(appDir).filePath(QStringLiteral("../../../config/database.ini"));
    paths.removeDuplicates();
    return paths;
}

bool DbConfig::load(DbConfig *config, QString *errorMessage)
{
    if (!config) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("数据库配置对象无效");
        }
        return false;
    }

    QString configPath;
    const QStringList paths = candidatePaths();
    for (const QString &path : paths) {
        if (QFile::exists(path)) {
            configPath = QDir::cleanPath(path);
            break;
        }
    }

    if (configPath.isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("未找到数据库配置文件。请创建 config/database.ini。\n尝试路径：\n%1")
                                .arg(paths.join(QLatin1Char('\n')));
        }
        return false;
    }

    QSettings settings(configPath, QSettings::IniFormat);
    settings.beginGroup(QStringLiteral("mysql"));
    config->host = settings.value(QStringLiteral("host"), config->host).toString().trimmed();
    config->port = settings.value(QStringLiteral("port"), config->port).toInt();
    config->databaseName = settings.value(QStringLiteral("database"), config->databaseName).toString().trimmed();
    config->username = settings.value(QStringLiteral("username"), config->username).toString().trimmed();
    config->password = settings.value(QStringLiteral("password"), config->password).toString();
    settings.endGroup();
    config->filePath = configPath;

    if (config->host.isEmpty() || config->databaseName.isEmpty() || config->username.isEmpty() || config->port <= 0) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("数据库配置不完整，请检查 %1 中的 host、port、database、username。").arg(configPath);
        }
        return false;
    }

    return true;
}
