#include "mainwindow.h"

#include "ui/agentpage.h"
#include "ui/coursepage.h"
#include "ui/dashboardpage.h"
#include "ui/notepage.h"
#include "ui/statspage.h"
#include "ui/taskpage.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

MainWindow::MainWindow(int userId, const QString &username, QWidget *parent)
    : QMainWindow(parent)
    , m_userId(userId)
    , m_username(username)
{
    setWindowTitle(QStringLiteral("智能学习任务管理系统"));
    resize(1180, 760);

    auto *central = new QWidget;
    auto *rootLayout = new QHBoxLayout(central);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    auto *sidebar = new QFrame;
    sidebar->setFixedWidth(190);
    sidebar->setStyleSheet(QStringLiteral("QFrame { background: #263238; color: white; } QPushButton { text-align: left; padding: 12px; border: none; color: white; } QPushButton:hover { background: #37474f; } QLabel { color: white; padding: 12px; }"));
    auto *sideLayout = new QVBoxLayout(sidebar);
    sideLayout->setContentsMargins(0, 12, 0, 12);

    auto *userLabel = new QLabel(QStringLiteral("你好，%1").arg(m_username));
    userLabel->setWordWrap(true);
    sideLayout->addWidget(userLabel);

    m_stack = new QStackedWidget;
    m_stack->addWidget(new DashboardPage(m_userId));
    m_stack->addWidget(new CoursePage(m_userId));
    m_stack->addWidget(new TaskPage(m_userId));
    m_stack->addWidget(new NotePage(m_userId));
    m_stack->addWidget(new StatsPage(m_userId));
    m_stack->addWidget(new AgentPage(m_userId));

    const QStringList labels = {
        QStringLiteral("首页"),
        QStringLiteral("课程管理"),
        QStringLiteral("任务管理"),
        QStringLiteral("学习笔记"),
        QStringLiteral("数据统计"),
        QStringLiteral("AI 助手")
    };
    for (int i = 0; i < labels.size(); ++i) {
        auto *button = new QPushButton(labels.at(i));
        connect(button, &QPushButton::clicked, this, [this, i]() {
            m_stack->setCurrentIndex(i);
        });
        sideLayout->addWidget(button);
    }

    sideLayout->addStretch();
    auto *logoutButton = new QPushButton(QStringLiteral("退出登录"));
    connect(logoutButton, &QPushButton::clicked, this, [this]() {
        close();
        emit loggedOut();
    });
    sideLayout->addWidget(logoutButton);

    rootLayout->addWidget(sidebar);
    rootLayout->addWidget(m_stack, 1);
    setCentralWidget(central);
}
