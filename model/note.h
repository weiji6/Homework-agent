#ifndef NOTE_H
#define NOTE_H

#include <QDateTime>
#include <QString>

struct Note
{
    int id = 0;
    int userId = 0;
    int courseId = 0;
    QString courseName;
    QString title;
    QString content;
    QDateTime createdAt;
    QDateTime updatedAt;
};

#endif // NOTE_H
