#ifndef COURSE_H
#define COURSE_H

#include <QString>

struct Course
{
    int id = 0;
    int userId = 0;
    QString name;
    QString teacher;
    QString classroom;
    int weekday = 1;
    QString startTime;
    QString endTime;
};

#endif // COURSE_H
