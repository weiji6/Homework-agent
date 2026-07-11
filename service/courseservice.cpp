#include "courseservice.h"

#include "db/dbmanager.h"
#include "db/sqlexecutor.h"

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
    const QString sql = QStringLiteral("SELECT id, user_id, name, teacher, classroom, weekday, start_time, end_time "
                                       "FROM course WHERE user_id = ? ORDER BY weekday, start_time, name");
    if (!SqlExecutor::exec(query, sql, {userId}, errorMessage)) {
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
    const QString sql = QStringLiteral("INSERT INTO course (user_id, name, teacher, classroom, weekday, start_time, end_time) "
                                       "VALUES (?, ?, ?, ?, ?, ?, ?)");
    const QVariantList params = {course.userId, course.name, course.teacher, course.classroom,
                                 course.weekday, course.startTime, course.endTime};
    if (!SqlExecutor::exec(query, sql, params, errorMessage)) {
        return false;
    }
    return true;
}

bool CourseService::updateCourse(const Course &course, QString *errorMessage)
{
    QSqlQuery query(DbManager::instance().database());
    const QString sql = QStringLiteral("UPDATE course SET name = ?, teacher = ?, classroom = ?, weekday = ?, start_time = ?, end_time = ? "
                                       "WHERE id = ? AND user_id = ?");
    const QVariantList params = {course.name, course.teacher, course.classroom, course.weekday,
                                 course.startTime, course.endTime, course.id, course.userId};
    if (!SqlExecutor::exec(query, sql, params, errorMessage)) {
        return false;
    }
    return true;
}

bool CourseService::removeCourse(int courseId, int userId, QString *errorMessage)
{
    QSqlQuery query(DbManager::instance().database());
    if (!SqlExecutor::exec(query,
                           QStringLiteral("DELETE FROM course WHERE id = ? AND user_id = ?"),
                           {courseId, userId},
                           errorMessage)) {
        return false;
    }
    return true;
}

QString CourseService::courseName(int courseId) const
{
    QSqlQuery query(DbManager::instance().database());
    if (SqlExecutor::exec(query, QStringLiteral("SELECT name FROM course WHERE id = ?"), {courseId}) && query.next()) {
        return query.value(0).toString();
    }
    return QString();
}
