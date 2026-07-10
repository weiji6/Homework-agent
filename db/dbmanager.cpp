#include "dbmanager.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QSettings>
#include <QStringList>

static QString resolveDriver(const QString &configuredDriver)
{
    const QString driver = configuredDriver.trimmed();
    const QStringList drivers = QSqlDatabase::drivers();
    if (driver.compare(QStringLiteral("auto"), Qt::CaseInsensitive) != 0 && drivers.contains(driver)) {
        return driver;
    }
    if (drivers.contains(QStringLiteral("QMYSQL"))) {
        return QStringLiteral("QMYSQL");
    }
    if (drivers.contains(QStringLiteral("QODBC"))) {
        return QStringLiteral("QODBC");
    }
    return driver;
}

static QString makeOdbcConnectionString(const QString &odbcDriver,
                                        const QString &host,
                                        int port,
                                        const QString &databaseName,
                                        const QString &username,
                                        const QString &password)
{
    return QStringLiteral("DRIVER={%1};SERVER=%2;PORT=%3;DATABASE=%4;UID=%5;PWD=%6;OPTION=3;")
        .arg(odbcDriver, host, QString::number(port), databaseName, username, password);
}

static QStringList installedOdbcDrivers()
{
    QStringList drivers;

    QSettings drivers64(QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\ODBC\\ODBCINST.INI\\ODBC Drivers"),
                        QSettings::NativeFormat);
    drivers << drivers64.allKeys();

    QSettings drivers32(QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\ODBC\\ODBCINST.INI\\ODBC Drivers"),
                        QSettings::NativeFormat);
    drivers << drivers32.allKeys();

    drivers.removeDuplicates();
    return drivers;
}

static QString resolveOdbcDriverName(const QString &configuredDriver)
{
    const QString configured = configuredDriver.trimmed();
    const QStringList drivers = installedOdbcDrivers();
    if (drivers.contains(configured)) {
        return configured;
    }

    QStringList mysqlDrivers;
    for (const QString &driver : drivers) {
        if (driver.contains(QStringLiteral("MySQL ODBC"), Qt::CaseInsensitive)) {
            mysqlDrivers << driver;
        }
    }

    for (const QString &driver : mysqlDrivers) {
        if (driver.contains(QStringLiteral("Unicode"), Qt::CaseInsensitive)) {
            return driver;
        }
    }
    if (!mysqlDrivers.isEmpty()) {
        return mysqlDrivers.first();
    }

    return configured;
}

DbManager &DbManager::instance()
{
    static DbManager manager;
    return manager;
}

DbManager::DbManager(QObject *parent)
    : QObject(parent)
{
}

bool DbManager::connectToDatabase(const QString &driver,
                                  const QString &host,
                                  int port,
                                  const QString &databaseName,
                                  const QString &username,
                                  const QString &password,
                                  const QString &odbcDriver,
                                  QString *errorMessage)
{
    const QString resolvedDriver = resolveDriver(driver);
    if (!QSqlDatabase::drivers().contains(resolvedDriver)) {
        m_lastError = QStringLiteral("当前 Qt 没有可用的 %1 驱动。可用驱动：%2")
                          .arg(resolvedDriver, QSqlDatabase::drivers().join(QStringLiteral(", ")));
        if (errorMessage) {
            *errorMessage = m_lastError;
        }
        return false;
    }

    const QString connectionName = QStringLiteral("smart_study_connection");
    if (QSqlDatabase::contains(connectionName)) {
        if (m_database.isOpen()) {
            m_database.close();
        }
        m_database = QSqlDatabase();
        QSqlDatabase::removeDatabase(connectionName);
    }

    m_database = QSqlDatabase::addDatabase(resolvedDriver, connectionName);

    if (resolvedDriver == QStringLiteral("QODBC")) {
        const QString actualOdbcDriver = resolveOdbcDriverName(odbcDriver);
        m_database.setDatabaseName(makeOdbcConnectionString(actualOdbcDriver, host, port, databaseName, username, password));
    } else {
        m_database.setHostName(host);
        m_database.setPort(port);
        m_database.setDatabaseName(databaseName);
        m_database.setUserName(username);
        m_database.setPassword(password);
        if (resolvedDriver == QStringLiteral("QMYSQL")) {
            m_database.setConnectOptions(QStringLiteral("MYSQL_OPT_RECONNECT=1"));
        }
    }

    if (!m_database.open()) {
        m_lastError = QStringLiteral("使用数据库驱动 %1 连接失败：%2\n已安装 ODBC 驱动：%3")
                          .arg(resolvedDriver,
                               m_database.lastError().text(),
                               installedOdbcDrivers().join(QStringLiteral(", ")));
        if (errorMessage) {
            *errorMessage = m_lastError;
        }
        return false;
    }

    return initializeSchema(errorMessage);
}

bool DbManager::initializeSchema(QString *errorMessage)
{
    if (!m_database.isOpen()) {
        m_lastError = QStringLiteral("数据库尚未连接");
        if (errorMessage) {
            *errorMessage = m_lastError;
        }
        return false;
    }

    const QStringList statements = {
        QStringLiteral("CREATE TABLE IF NOT EXISTS `user` ("
                       "id INT PRIMARY KEY AUTO_INCREMENT,"
                       "username VARCHAR(50) NOT NULL UNIQUE,"
                       "password VARCHAR(100) NOT NULL,"
                       "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
                       ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4"),
        QStringLiteral("CREATE TABLE IF NOT EXISTS course ("
                       "id INT PRIMARY KEY AUTO_INCREMENT,"
                       "user_id INT NOT NULL,"
                       "name VARCHAR(100) NOT NULL,"
                       "teacher VARCHAR(100),"
                       "classroom VARCHAR(100),"
                       "weekday INT,"
                       "start_time VARCHAR(20),"
                       "end_time VARCHAR(20),"
                       "FOREIGN KEY (user_id) REFERENCES `user`(id) ON DELETE CASCADE"
                       ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4"),
        QStringLiteral("CREATE TABLE IF NOT EXISTS task ("
                       "id INT PRIMARY KEY AUTO_INCREMENT,"
                       "user_id INT NOT NULL,"
                       "course_id INT,"
                       "title VARCHAR(200) NOT NULL,"
                       "type VARCHAR(50),"
                       "content TEXT,"
                       "deadline DATETIME,"
                       "status VARCHAR(20) DEFAULT '未完成',"
                       "priority VARCHAR(20) DEFAULT '普通',"
                       "created_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
                       "FOREIGN KEY (user_id) REFERENCES `user`(id) ON DELETE CASCADE,"
                       "FOREIGN KEY (course_id) REFERENCES course(id) ON DELETE SET NULL"
                       ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4"),
        QStringLiteral("CREATE TABLE IF NOT EXISTS note ("
                       "id INT PRIMARY KEY AUTO_INCREMENT,"
                       "user_id INT NOT NULL,"
                       "course_id INT,"
                       "title VARCHAR(200) NOT NULL,"
                       "content TEXT,"
                       "created_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
                       "updated_at DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,"
                       "FOREIGN KEY (user_id) REFERENCES `user`(id) ON DELETE CASCADE,"
                       "FOREIGN KEY (course_id) REFERENCES course(id) ON DELETE SET NULL"
                       ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4")
    };

    QSqlQuery query(m_database);
    for (const QString &statement : statements) {
        if (!query.exec(statement)) {
            m_lastError = query.lastError().text();
            if (errorMessage) {
                *errorMessage = m_lastError;
            }
            return false;
        }
    }

    return true;
}

bool DbManager::isOpen() const
{
    return m_database.isOpen();
}

QSqlDatabase DbManager::database() const
{
    return m_database;
}

QString DbManager::lastError() const
{
    return m_lastError;
}
