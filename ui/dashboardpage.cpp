#include "dashboardpage.h"

#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>

static QGroupBox *makeMetricBox(const QString &title, QLabel **valueLabel)
{
    auto *box = new QGroupBox(title);
    auto *layout = new QVBoxLayout(box);
    layout->setContentsMargins(18, 20, 18, 18);
    layout->setSpacing(8);

    *valueLabel = new QLabel(QStringLiteral("0"));
    QFont font = (*valueLabel)->font();
    font.setPointSize(30);
    font.setBold(true);
    (*valueLabel)->setFont(font);
    (*valueLabel)->setAlignment(Qt::AlignCenter);
    layout->addWidget(*valueLabel);
    return box;
}

DashboardPage::DashboardPage(int userId, QWidget *parent)
    : QWidget(parent)
    , m_userId(userId)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(28, 24, 28, 24);
    layout->setSpacing(18);

    auto *top = new QHBoxLayout;
    auto *titleBox = new QVBoxLayout;
    auto *title = new QLabel(QStringLiteral("首页仪表盘"));
    title->setObjectName(QStringLiteral("PageTitle"));
    auto *subtitle = new QLabel(QStringLiteral("快速查看课程任务状态，及时处理临近截止和逾期事项。"));
    subtitle->setObjectName(QStringLiteral("MutedText"));
    subtitle->setWordWrap(true);
    titleBox->addWidget(title);
    titleBox->addWidget(subtitle);

    auto *refreshButton = new QPushButton(QStringLiteral("刷新"));
    refreshButton->setProperty("primary", true);
    connect(refreshButton, &QPushButton::clicked, this, &DashboardPage::refresh);
    top->addLayout(titleBox);
    top->addStretch();
    top->addWidget(refreshButton);

    auto *grid = new QGridLayout;
    grid->setSpacing(16);
    grid->addWidget(makeMetricBox(QStringLiteral("总任务数"), &m_totalLabel), 0, 0);
    grid->addWidget(makeMetricBox(QStringLiteral("已完成"), &m_doneLabel), 0, 1);
    grid->addWidget(makeMetricBox(QStringLiteral("未完成"), &m_pendingLabel), 1, 0);
    grid->addWidget(makeMetricBox(QStringLiteral("逾期任务"), &m_overdueLabel), 1, 1);

    layout->addLayout(top);
    layout->addLayout(grid);
    layout->addStretch();
    refresh();
}

void DashboardPage::refresh()
{
    m_totalLabel->setText(QString::number(m_taskService.countTasks(m_userId)));
    m_doneLabel->setText(QString::number(m_taskService.countTasks(m_userId, QStringLiteral("已完成"))));
    m_pendingLabel->setText(QString::number(m_taskService.countTasks(m_userId, QStringLiteral("未完成"))));
    m_overdueLabel->setText(QString::number(m_taskService.overdueCount(m_userId)));
}
