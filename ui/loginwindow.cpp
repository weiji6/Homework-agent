#include "loginwindow.h"

#include "db/dbconfig.h"
#include "db/dbmanager.h"
#include "ui/mainwindow.h"

#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

LoginWindow::LoginWindow(QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("LoginWindow"));
    setWindowTitle(QStringLiteral("智能学习任务管理系统 - 登录"));
    resize(520, 420);

    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(48, 42, 48, 42);
    rootLayout->setSpacing(0);

    auto *card = new QFrame;
    card->setObjectName(QStringLiteral("LoginCard"));
    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(34, 32, 34, 30);
    cardLayout->setSpacing(18);

    auto *title = new QLabel(QStringLiteral("智能学习任务管理系统"));
    title->setObjectName(QStringLiteral("AppTitle"));
    title->setAlignment(Qt::AlignCenter);

    m_usernameEdit = new QLineEdit;
    m_usernameEdit->setPlaceholderText(QStringLiteral("请输入用户名"));
    m_passwordEdit = new QLineEdit;
    m_passwordEdit->setPlaceholderText(QStringLiteral("请输入密码"));
    m_passwordEdit->setEchoMode(QLineEdit::Password);

    auto *accountForm = new QFormLayout;
    accountForm->setLabelAlignment(Qt::AlignRight);
    accountForm->setFormAlignment(Qt::AlignHCenter);
    accountForm->setHorizontalSpacing(14);
    accountForm->setVerticalSpacing(14);
    accountForm->addRow(QStringLiteral("用户名"), m_usernameEdit);
    accountForm->addRow(QStringLiteral("密码"), m_passwordEdit);

    auto *loginButton = new QPushButton(QStringLiteral("登录"));
    loginButton->setProperty("primary", true);
    auto *registerButton = new QPushButton(QStringLiteral("注册账号"));
    connect(loginButton, &QPushButton::clicked, this, &LoginWindow::login);
    connect(registerButton, &QPushButton::clicked, this, &LoginWindow::registerAccount);

    auto *buttonLayout = new QHBoxLayout;
    buttonLayout->setSpacing(12);
    buttonLayout->addWidget(loginButton);
    buttonLayout->addWidget(registerButton);

    cardLayout->addWidget(title);
    cardLayout->addSpacing(10);
    cardLayout->addLayout(accountForm);
    cardLayout->addSpacing(6);
    cardLayout->addLayout(buttonLayout);

    rootLayout->addStretch();
    rootLayout->addWidget(card);
    rootLayout->addStretch();
}

void LoginWindow::login()
{
    if (!ensureDatabase()) {
        return;
    }

    int userId = 0;
    QString error;
    if (!m_userService.login(m_usernameEdit->text(), m_passwordEdit->text(), &userId, &error)) {
        QMessageBox::warning(this, QStringLiteral("登录失败"), error);
        return;
    }

    openMainWindow(userId, m_usernameEdit->text().trimmed());
}

void LoginWindow::registerAccount()
{
    if (!ensureDatabase()) {
        return;
    }

    QString error;
    if (!m_userService.registerUser(m_usernameEdit->text(), m_passwordEdit->text(), &error)) {
        QMessageBox::warning(this, QStringLiteral("注册失败"), error);
        return;
    }

    QMessageBox::information(this, QStringLiteral("注册成功"), QStringLiteral("账号已创建，可以直接登录。"));
}

bool LoginWindow::ensureDatabase()
{
    if (DbManager::instance().isOpen()) {
        return true;
    }

    QString error;
    DbConfig config;
    if (!DbConfig::load(&config, &error)) {
        QMessageBox::critical(this, QStringLiteral("数据库配置错误"), error);
        return false;
    }

    const bool ok = DbManager::instance().connectToDatabase(config.driver,
                                                            config.host,
                                                            config.port,
                                                            config.databaseName,
                                                            config.username,
                                                            config.password,
                                                            config.odbcDriver,
                                                            &error);
    if (!ok) {
        QMessageBox::critical(this,
                              QStringLiteral("数据库连接失败"),
                              QStringLiteral("请确认 MySQL 已启动、数据库已创建、Qt 已安装 QMYSQL 驱动。\n\n错误：%1")
                                  .arg(error));
    }
    return ok;
}

void LoginWindow::openMainWindow(int userId, const QString &username)
{
    auto *mainWindow = new MainWindow(userId, username);
    connect(mainWindow, &MainWindow::loggedOut, this, [this, mainWindow]() {
        mainWindow->deleteLater();
        show();
    });
    mainWindow->show();
    hide();
}
