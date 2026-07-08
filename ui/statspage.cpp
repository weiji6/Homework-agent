#include "statspage.h"

#include "db/dbmanager.h"

#include <QHeaderView>
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
    auto *refreshButton = new QPushButton(QStringLiteral("刷新统计"));
    connect(refreshButton, &QPushButton::clicked, this, &StatsPage::refresh);
    m_statusTable = new QTableWidget;
    m_courseTable = new QTableWidget;
    m_priorityTable = new QTableWidget;
    layout->addWidget(refreshButton);
    layout->addWidget(new QLabel(QStringLiteral("按状态统计")));
    layout->addWidget(m_statusTable);
    layout->addWidget(new QLabel(QStringLiteral("每门课程任务数量")));
    layout->addWidget(m_courseTable);
    layout->addWidget(new QLabel(QStringLiteral("按优先级统计")));
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
    while (query.next()) {
        const int row = table->rowCount();
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
        table->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));
    }
    table->horizontalHeader()->setStretchLastSection(true);
}
