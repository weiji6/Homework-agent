#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include "service/userservice.h"

#include <QLineEdit>
#include <QWidget>

class LoginWindow : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);

private slots:
    void login();
    void registerAccount();

private:
    bool ensureDatabase();
    void openMainWindow(int userId, const QString &username);

    QLineEdit *m_hostEdit = nullptr;
    QLineEdit *m_portEdit = nullptr;
    QLineEdit *m_databaseEdit = nullptr;
    QLineEdit *m_dbUserEdit = nullptr;
    QLineEdit *m_dbPasswordEdit = nullptr;
    QLineEdit *m_usernameEdit = nullptr;
    QLineEdit *m_passwordEdit = nullptr;
    UserService m_userService;
};

#endif // LOGINWINDOW_H
