#ifndef TASK_H
#define TASK_H

#include <QDateTime>
#include <QString>

struct StudyTask
{
    int id = 0;
    int userId = 0;
    int courseId = 0;
    QString courseName;
    QString title;
    QString type;
    QString content;
    QDateTime deadline;
    QString status = QStringLiteral("未完成");
    QString priority = QStringLiteral("普通");
    QDateTime createdAt;
};

#endif // TASK_H
