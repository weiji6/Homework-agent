#include "loginwindow.h"

#include "db/dbmanager.h"
#include "ui/mainwindow.h"

#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

LoginWindow::LoginWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle(QStringLiteral("智能学习任务管理系统 - 登录"));
    resize(460, 520);

    auto *title = new QLabel(QStringLiteral("智能学习任务管理系统"));
    QFont titleFont = title->font();
    titleFont.setPointSize(20);
    titleFont.setBold(true);
    title->setFont(titleFont);
    title->setAlignment(Qt::AlignCenter);

    m_usernameEdit = new QLineEdit;
    m_usernameEdit->setPlaceholderText(QStringLiteral("请输入用户名"));
    m_passwordEdit = new QLineEdit;
    m_passwordEdit->setPlaceholderText(QStringLiteral("请输入密码"));
    m_passwordEdit->setEchoMode(QLineEdit::Password);

    auto *accountForm = new QFormLayout;
    accountForm->addRow(QStringLiteral("用户名"), m_usernameEdit);
    accountForm->addRow(QStringLiteral("密码"), m_passwordEdit);

    auto *loginButton = new QPushButton(QStringLiteral("登录"));
    auto *registerButton = new QPushButton(QStringLiteral("注册账号"));
    connect(loginButton, &QPushButton::clicked, this, &LoginWindow::login);
    connect(registerButton, &QPushButton::clicked, this, &LoginWindow::registerAccount);

    auto *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(loginButton);
    buttonLayout->addWidget(registerButton);

    m_hostEdit = new QLineEdit(QStringLiteral("127.0.0.1"));
    m_portEdit = new QLineEdit(QStringLiteral("3306"));
    m_databaseEdit = new QLineEdit(QStringLiteral("smart_study"));
    m_dbUserEdit = new QLineEdit(QStringLiteral("root"));
    m_dbPasswordEdit = new QLineEdit;
    m_dbPasswordEdit->setEchoMode(QLineEdit::Password);

    auto *dbForm = new QFormLayout;
    dbForm->addRow(QStringLiteral("主机"), m_hostEdit);
    dbForm->addRow(QStringLiteral("端口"), m_portEdit);
    dbForm->addRow(QStringLiteral("数据库"), m_databaseEdit);
    dbForm->addRow(QStringLiteral("数据库用户"), m_dbUserEdit);
    dbForm->addRow(QStringLiteral("数据库密码"), m_dbPasswordEdit);

    auto *dbBox = new QGroupBox(QStringLiteral("MySQL 连接配置"));
    dbBox->setLayout(dbForm);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(title);
    layout->addSpacing(18);
    layout->addLayout(accountForm);
    layout->addLayout(buttonLayout);
    layout->addSpacing(12);
    layout->addWidget(dbBox);
    layout->addStretch();
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
    QString error;
    const bool ok = DbManager::instance().connectToDatabase(m_hostEdit->text().trimmed(),
                                                            m_portEdit->text().toInt(),
                                                            m_databaseEdit->text().trimmed(),
                                                            m_dbUserEdit->text().trimmed(),
                                                            m_dbPasswordEdit->text(),
                                                            &error);
    if (!ok) {
        QMessageBox::critical(this,
                              QStringLiteral("数据库连接失败"),
                              QStringLiteral("请确认 MySQL 已启动、数据库已创建，并且 Qt 安装了 QMYSQL 驱动。\n\n错误：%1").arg(error));
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
