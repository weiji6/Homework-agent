#ifndef TASKSERVICE_H
#define TASKSERVICE_H

#include "model/task.h"

#include <QObject>
#include <QVariant>
#include <QVector>

class QSqlQuery;

class TaskService : public QObject
{
    Q_OBJECT

public:
    explicit TaskService(QObject *parent = nullptr);

    QVector<StudyTask> listTasks(int userId,
                                 const QString &status = QString(),
                                 int courseId = 0,
                                 QString *errorMessage = nullptr) const;
    QVector<StudyTask> tasksDueInDays(int userId, int days, QString *errorMessage = nullptr) const;
    QVector<StudyTask> overdueTasks(int userId, QString *errorMessage = nullptr) const;
    bool addTask(const StudyTask &task, QString *errorMessage = nullptr);
    bool updateTask(const StudyTask &task, QString *errorMessage = nullptr);
    bool removeTask(int taskId, int userId, QString *errorMessage = nullptr);
    bool markCompleted(int taskId, int userId, QString *errorMessage = nullptr);
    int countTasks(int userId, const QString &status = QString()) const;
    int overdueCount(int userId) const;

private:
    QVector<StudyTask> readTasks(QSqlQuery &query,
                                 const QString &sql,
                                 const QVariantList &params,
                                 QString *errorMessage) const;
};

#endif // TASKSERVICE_H
