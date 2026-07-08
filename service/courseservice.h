#ifndef COURSESERVICE_H
#define COURSESERVICE_H

#include "model/course.h"

#include <QObject>
#include <QVector>

class CourseService : public QObject
{
    Q_OBJECT

public:
    explicit CourseService(QObject *parent = nullptr);

    QVector<Course> listCourses(int userId, QString *errorMessage = nullptr) const;
    bool addCourse(const Course &course, QString *errorMessage = nullptr);
    bool updateCourse(const Course &course, QString *errorMessage = nullptr);
    bool removeCourse(int courseId, int userId, QString *errorMessage = nullptr);
    QString courseName(int courseId) const;
};

#endif // COURSESERVICE_H
