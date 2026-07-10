#include "mainwindow.h"

#include "ui/agentpage.h"
#include "ui/coursepage.h"
#include "ui/dashboardpage.h"
#include "ui/notepage.h"
#include "ui/statspage.h"
#include "ui/taskpage.h"

#include <QButtonGroup>
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
    resize(1220, 780);

    auto *central = new QWidget;
    auto *rootLayout = new QHBoxLayout(central);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    auto *sidebar = new QFrame;
    sidebar->setObjectName(QStringLiteral("Sidebar"));
    sidebar->setFixedWidth(224);
    auto *sideLayout = new QVBoxLayout(sidebar);
    sideLayout->setContentsMargins(18, 22, 18, 18);
    sideLayout->setSpacing(10);

    auto *appTitle = new QLabel(QStringLiteral("学习管家"));
    appTitle->setObjectName(QStringLiteral("SidebarTitle"));
    auto *userLabel = new QLabel(QStringLiteral("你好，%1").arg(m_username));
    userLabel->setObjectName(QStringLiteral("SidebarUser"));
    userLabel->setWordWrap(true);
    sideLayout->addWidget(appTitle);
    sideLayout->addWidget(userLabel);
    sideLayout->addSpacing(16);

    m_stack = new QStackedWidget;
    m_stack->addWidget(new DashboardPage(m_userId));
    m_stack->addWidget(new CoursePage(m_userId));
    m_stack->addWidget(new TaskPage(m_userId));
    m_stack->addWidget(new NotePage(m_userId));
    m_stack->addWidget(new StatsPage(m_userId));
    m_stack->addWidget(new AgentPage(m_userId));

    auto *navGroup = new QButtonGroup(this);
    navGroup->setExclusive(true);
    const QStringList labels = {
        QStringLiteral("首页仪表盘"),
        QStringLiteral("课程管理"),
        QStringLiteral("任务管理"),
        QStringLiteral("学习笔记"),
        QStringLiteral("数据统计"),
        QStringLiteral("AI 助手")
    };
    for (int i = 0; i < labels.size(); ++i) {
        auto *button = new QPushButton(labels.at(i));
        button->setCheckable(true);
        button->setCursor(Qt::PointingHandCursor);
        navGroup->addButton(button, i);
        sideLayout->addWidget(button);
    }
    if (auto *firstButton = navGroup->button(0)) {
        firstButton->setChecked(true);
    }
    connect(navGroup, &QButtonGroup::idClicked, this, [this](int index) {
        m_stack->setCurrentIndex(index);
    });

    sideLayout->addStretch();
    auto *logoutButton = new QPushButton(QStringLiteral("退出登录"));
    logoutButton->setCursor(Qt::PointingHandCursor);
    connect(logoutButton, &QPushButton::clicked, this, [this]() {
        close();
        emit loggedOut();
    });
    sideLayout->addWidget(logoutButton);

    rootLayout->addWidget(sidebar);
    rootLayout->addWidget(m_stack, 1);
    setCentralWidget(central);
}
