#include "userservice.h"

#include "db/dbmanager.h"

#include <QCryptographicHash>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

UserService::UserService(QObject *parent)
    : QObject(parent)
{
}

bool UserService::registerUser(const QString &username, const QString &password, QString *errorMessage)
{
    if (username.trimmed().isEmpty() || password.length() < 4) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("用户名不能为空，密码至少 4 位");
        }
        return false;
    }

    QSqlQuery query(DbManager::instance().database());
    query.prepare(QStringLiteral("INSERT INTO `user` (username, password) VALUES (?, ?)"));
    query.addBindValue(username.trimmed());
    query.addBindValue(hashPassword(password));
    if (!query.exec()) {
        if (errorMessage) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }
    return true;
}

bool UserService::login(const QString &username, const QString &password, int *userId, QString *errorMessage)
{
    QSqlQuery query(DbManager::instance().database());
    query.prepare(QStringLiteral("SELECT id FROM `user` WHERE username = ? AND password = ?"));
    query.addBindValue(username.trimmed());
    query.addBindValue(hashPassword(password));
    if (!query.exec()) {
        if (errorMessage) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }

    if (!query.next()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("用户名或密码错误");
        }
        return false;
    }

    if (userId) {
        *userId = query.value(0).toInt();
    }
    return true;
}

bool UserService::changePassword(int userId, const QString &oldPassword, const QString &newPassword, QString *errorMessage)
{
    if (newPassword.length() < 4) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("新密码至少 4 位");
        }
        return false;
    }

    QSqlQuery query(DbManager::instance().database());
    query.prepare(QStringLiteral("UPDATE `user` SET password = ? WHERE id = ? AND password = ?"));
    query.addBindValue(hashPassword(newPassword));
    query.addBindValue(userId);
    query.addBindValue(hashPassword(oldPassword));
    if (!query.exec()) {
        if (errorMessage) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }

    if (query.numRowsAffected() == 0) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("原密码不正确");
        }
        return false;
    }
    return true;
}

QString UserService::hashPassword(const QString &password) const
{
    const QByteArray data = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex();
    return QString::fromLatin1(data);
}
