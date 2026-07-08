#include "dbmanager.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>

DbManager &DbManager::instance()
{
    static DbManager manager;
    return manager;
}

DbManager::DbManager(QObject *parent)
    : QObject(parent)
{
}

bool DbManager::connectToDatabase(const QString &host,
                                  int port,
                                  const QString &databaseName,
                                  const QString &username,
                                  const QString &password,
                                  QString *errorMessage)
{
    const QString connectionName = QStringLiteral("smart_study_connection");
    if (QSqlDatabase::contains(connectionName)) {
        m_database = QSqlDatabase::database(connectionName);
    } else {
        m_database = QSqlDatabase::addDatabase(QStringLiteral("QMYSQL"), connectionName);
    }

    if (m_database.isOpen()) {
        m_database.close();
    }

    m_database.setHostName(host);
    m_database.setPort(port);
    m_database.setDatabaseName(databaseName);
    m_database.setUserName(username);
    m_database.setPassword(password);
    m_database.setConnectOptions(QStringLiteral("MYSQL_OPT_RECONNECT=1"));

    if (!m_database.open()) {
        m_lastError = m_database.lastError().text();
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
