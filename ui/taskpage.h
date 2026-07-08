#ifndef TASKPAGE_H
#define TASKPAGE_H

#include "model/task.h"
#include "service/courseservice.h"
#include "service/taskservice.h"

#include <QComboBox>
#include <QTableWidget>
#include <QWidget>

class TaskPage : public QWidget
{
    Q_OBJECT

public:
    explicit TaskPage(int userId, QWidget *parent = nullptr);

private slots:
    void refresh();
    void addTask();
    void editTask();
    void deleteTask();
    void markDone();

private:
    StudyTask currentTask() const;
    bool editDialog(StudyTask *task, const QString &title);

    int m_userId = 0;
    QComboBox *m_statusFilter = nullptr;
    QComboBox *m_courseFilter = nullptr;
    QTableWidget *m_table = nullptr;
    TaskService m_taskService;
    CourseService m_courseService;
};

#endif // TASKPAGE_H
