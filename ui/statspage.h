#ifndef STATSPAGE_H
#define STATSPAGE_H

#include <QTableWidget>
#include <QWidget>

class StatsPage : public QWidget
{
    Q_OBJECT

public:
    explicit StatsPage(int userId, QWidget *parent = nullptr);

private slots:
    void refresh();

private:
    void fillQueryTable(QTableWidget *table, const QString &sql, const QVariantList &params);

    int m_userId = 0;
    QTableWidget *m_statusTable = nullptr;
    QTableWidget *m_courseTable = nullptr;
    QTableWidget *m_priorityTable = nullptr;
};

#endif // STATSPAGE_H
