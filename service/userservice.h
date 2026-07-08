#ifndef USERSERVICE_H
#define USERSERVICE_H

#include <QObject>
#include <QString>

class UserService : public QObject
{
    Q_OBJECT

public:
    explicit UserService(QObject *parent = nullptr);

    bool registerUser(const QString &username, const QString &password, QString *errorMessage = nullptr);
    bool login(const QString &username, const QString &password, int *userId, QString *errorMessage = nullptr);
    bool changePassword(int userId, const QString &oldPassword, const QString &newPassword, QString *errorMessage = nullptr);

private:
    QString hashPassword(const QString &password) const;
};

#endif // USERSERVICE_H
