#include "courseservice.h"

#include "db/dbmanager.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

CourseService::CourseService(QObject *parent)
    : QObject(parent)
{
}

QVector<Course> CourseService::listCourses(int userId, QString *errorMessage) const
{
    QVector<Course> courses;
    QSqlQuery query(DbManager::instance().database());
    query.prepare(QStringLiteral("SELECT id, user_id, name, teacher, classroom, weekday, start_time, end_time "
                                 "FROM course WHERE user_id = ? ORDER BY weekday, start_time, name"));
    query.addBindValue(userId);
    if (!query.exec()) {
        if (errorMessage) {
            *errorMessage = query.lastError().text();
        }
        return courses;
    }

    while (query.next()) {
        Course course;
        course.id = query.value(0).toInt();
        course.userId = query.value(1).toInt();
        course.name = query.value(2).toString();
        course.teacher = query.value(3).toString();
        course.classroom = query.value(4).toString();
        course.weekday = query.value(5).toInt();
        course.startTime = query.value(6).toString();
        course.endTime = query.value(7).toString();
        courses.append(course);
    }
    return courses;
}

bool CourseService::addCourse(const Course &course, QString *errorMessage)
{
    QSqlQuery query(DbManager::instance().database());
    query.prepare(QStringLiteral("INSERT INTO course (user_id, name, teacher, classroom, weekday, start_time, end_time) "
                                 "VALUES (?, ?, ?, ?, ?, ?, ?)"));
    query.addBindValue(course.userId);
    query.addBindValue(course.name);
    query.addBindValue(course.teacher);
    query.addBindValue(course.classroom);
    query.addBindValue(course.weekday);
    query.addBindValue(course.startTime);
    query.addBindValue(course.endTime);
    if (!query.exec()) {
        if (errorMessage) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }
    return true;
}

bool CourseService::updateCourse(const Course &course, QString *errorMessage)
{
    QSqlQuery query(DbManager::instance().database());
    query.prepare(QStringLiteral("UPDATE course SET name = ?, teacher = ?, classroom = ?, weekday = ?, start_time = ?, end_time = ? "
                                 "WHERE id = ? AND user_id = ?"));
    query.addBindValue(course.name);
    query.addBindValue(course.teacher);
    query.addBindValue(course.classroom);
    query.addBindValue(course.weekday);
    query.addBindValue(course.startTime);
    query.addBindValue(course.endTime);
    query.addBindValue(course.id);
    query.addBindValue(course.userId);
    if (!query.exec()) {
        if (errorMessage) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }
    return true;
}

bool CourseService::removeCourse(int courseId, int userId, QString *errorMessage)
{
    QSqlQuery query(DbManager::instance().database());
    query.prepare(QStringLiteral("DELETE FROM course WHERE id = ? AND user_id = ?"));
    query.addBindValue(courseId);
    query.addBindValue(userId);
    if (!query.exec()) {
        if (errorMessage) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }
    return true;
}

QString CourseService::courseName(int courseId) const
{
    QSqlQuery query(DbManager::instance().database());
    query.prepare(QStringLiteral("SELECT name FROM course WHERE id = ?"));
    query.addBindValue(courseId);
    if (query.exec() && query.next()) {
        return query.value(0).toString();
    }
    return QString();
}
