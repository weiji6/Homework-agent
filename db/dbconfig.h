#ifndef DBCONFIG_H
#define DBCONFIG_H

#include <QString>
#include <QStringList>

struct DbConfig
{
    QString driver = QStringLiteral("auto");
    QString host = QStringLiteral("127.0.0.1");
    int port = 3306;
    QString databaseName = QStringLiteral("smart_study");
    QString username = QStringLiteral("root");
    QString password;
    QString odbcDriver = QStringLiteral("MySQL ODBC 8.0 Unicode Driver");
    QString filePath;

    static bool load(DbConfig *config, QString *errorMessage = nullptr);
    static QStringList candidatePaths();
};

#endif // DBCONFIG_H
