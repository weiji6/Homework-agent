#include "statspage.h"

#include "db/dbmanager.h"

#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSqlQuery>
#include <QVariant>
#include <QVBoxLayout>

StatsPage::StatsPage(int userId, QWidget *parent)
    : QWidget(parent)
    , m_userId(userId)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(28, 24, 28, 24);
    layout->setSpacing(14);

    auto *top = new QHBoxLayout;
    auto *titleBox = new QVBoxLayout;
    auto *title = new QLabel(QStringLiteral("数据统计"));
    title->setObjectName(QStringLiteral("PageTitle"));
    auto *subtitle = new QLabel(QStringLiteral("从状态、课程和优先级三个维度查看任务分布。"));
    subtitle->setObjectName(QStringLiteral("MutedText"));
    titleBox->addWidget(title);
    titleBox->addWidget(subtitle);

    auto *refreshButton = new QPushButton(QStringLiteral("刷新统计"));
    refreshButton->setProperty("primary", true);
    connect(refreshButton, &QPushButton::clicked, this, &StatsPage::refresh);
    top->addLayout(titleBox);
    top->addStretch();
    top->addWidget(refreshButton);

    m_statusTable = new QTableWidget;
    m_courseTable = new QTableWidget;
    m_priorityTable = new QTableWidget;

    auto *statusLabel = new QLabel(QStringLiteral("按状态统计"));
    statusLabel->setObjectName(QStringLiteral("SectionTitle"));
    auto *courseLabel = new QLabel(QStringLiteral("每门课程任务数量"));
    courseLabel->setObjectName(QStringLiteral("SectionTitle"));
    auto *priorityLabel = new QLabel(QStringLiteral("按优先级统计"));
    priorityLabel->setObjectName(QStringLiteral("SectionTitle"));

    layout->addLayout(top);
    layout->addWidget(statusLabel);
    layout->addWidget(m_statusTable);
    layout->addWidget(courseLabel);
    layout->addWidget(m_courseTable);
    layout->addWidget(priorityLabel);
    layout->addWidget(m_priorityTable);
    refresh();
}

void StatsPage::refresh()
{
    fillQueryTable(m_statusTable,
                   QStringLiteral("SELECT status, COUNT(*) FROM task WHERE user_id = ? GROUP BY status"),
                   {m_userId});
    fillQueryTable(m_courseTable,
                   QStringLiteral("SELECT c.name, COUNT(t.id) FROM course c LEFT JOIN task t ON c.id = t.course_id WHERE c.user_id = ? GROUP BY c.id, c.name"),
                   {m_userId});
    fillQueryTable(m_priorityTable,
                   QStringLiteral("SELECT priority, COUNT(*) FROM task WHERE user_id = ? GROUP BY priority"),
                   {m_userId});
}

void StatsPage::fillQueryTable(QTableWidget *table, const QString &sql, const QVariantList &params)
{
    QSqlQuery query(DbManager::instance().database());
    query.prepare(sql);
    for (const QVariant &param : params) {
        query.addBindValue(param);
    }
    query.exec();

    table->clear();
    table->setColumnCount(2);
    table->setHorizontalHeaderLabels({QStringLiteral("分类"), QStringLiteral("数量")});
    table->setRowCount(0);
    table->verticalHeader()->setVisible(false);
    table->verticalHeader()->setDefaultSectionSize(36);
    table->setAlternatingRowColors(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    while (query.next()) {
        const int row = table->rowCount();
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
        table->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));
    }
    table->horizontalHeader()->setStretchLastSection(true);
}
