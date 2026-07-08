#include "dashboardpage.h"

#include <QGridLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QVBoxLayout>

static QGroupBox *makeMetricBox(const QString &title, QLabel **valueLabel)
{
    auto *box = new QGroupBox(title);
    auto *layout = new QVBoxLayout(box);
    *valueLabel = new QLabel(QStringLiteral("0"));
    QFont font = (*valueLabel)->font();
    font.setPointSize(24);
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
    auto *top = new QHBoxLayout;
    auto *title = new QLabel(QStringLiteral("首页仪表盘"));
    QFont titleFont = title->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    title->setFont(titleFont);
    auto *refreshButton = new QPushButton(QStringLiteral("刷新"));
    connect(refreshButton, &QPushButton::clicked, this, &DashboardPage::refresh);
    top->addWidget(title);
    top->addStretch();
    top->addWidget(refreshButton);

    auto *grid = new QGridLayout;
    grid->addWidget(makeMetricBox(QStringLiteral("总任务数"), &m_totalLabel), 0, 0);
    grid->addWidget(makeMetricBox(QStringLiteral("已完成"), &m_doneLabel), 0, 1);
    grid->addWidget(makeMetricBox(QStringLiteral("未完成"), &m_pendingLabel), 1, 0);
    grid->addWidget(makeMetricBox(QStringLiteral("逾期任务"), &m_overdueLabel), 1, 1);

    auto *hint = new QLabel(QStringLiteral("提示：任务、课程、笔记页面中的数据都会写入 MySQL，AI 助手会基于这些数据回答问题。"));
    hint->setWordWrap(true);

    layout->addLayout(top);
    layout->addLayout(grid);
    layout->addWidget(hint);
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
