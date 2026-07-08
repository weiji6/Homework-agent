#ifndef USER_H
#define USER_H

#include <QDateTime>
#include <QString>

struct User
{
    int id = 0;
    QString username;
    QDateTime createdAt;
};

#endif // USER_H
