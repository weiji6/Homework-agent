#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QString>

class DbManager : public QObject
{
    Q_OBJECT

public:
    static DbManager &instance();

    bool connectToDatabase(const QString &driver,
                           const QString &host,
                           int port,
                           const QString &databaseName,
                           const QString &username,
                           const QString &password,
                           const QString &odbcDriver,
                           QString *errorMessage = nullptr);
    bool initializeSchema(QString *errorMessage = nullptr);
    bool isOpen() const;
    QSqlDatabase database() const;
    QString lastError() const;

private:
    explicit DbManager(QObject *parent = nullptr);

    QSqlDatabase m_database;
    QString m_lastError;
};

#endif // DBMANAGER_H
