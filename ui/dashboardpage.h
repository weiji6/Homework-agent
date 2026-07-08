#ifndef DASHBOARDPAGE_H
#define DASHBOARDPAGE_H

#include "service/taskservice.h"

#include <QLabel>
#include <QWidget>

class DashboardPage : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardPage(int userId, QWidget *parent = nullptr);

private slots:
    void refresh();

private:
    int m_userId = 0;
    QLabel *m_totalLabel = nullptr;
    QLabel *m_doneLabel = nullptr;
    QLabel *m_pendingLabel = nullptr;
    QLabel *m_overdueLabel = nullptr;
    TaskService m_taskService;
};

#endif // DASHBOARDPAGE_H
