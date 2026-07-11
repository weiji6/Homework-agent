#include "taskservice.h"

#include "db/dbmanager.h"
#include "db/sqlexecutor.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

TaskService::TaskService(QObject *parent)
    : QObject(parent)
{
}

QVector<StudyTask> TaskService::listTasks(int userId, const QString &status, int courseId, QString *errorMessage) const
{
    QString sql = QStringLiteral("SELECT t.id, t.user_id, IFNULL(t.course_id, 0), IFNULL(c.name, ''), "
                                 "t.title, t.type, t.content, t.deadline, t.status, t.priority, t.created_at "
                                 "FROM task t LEFT JOIN course c ON t.course_id = c.id "
                                 "WHERE t.user_id = ?");
    if (!status.isEmpty()) {
        sql += QStringLiteral(" AND t.status = ?");
    }
    if (courseId > 0) {
        sql += QStringLiteral(" AND t.course_id = ?");
    }
    sql += QStringLiteral(" ORDER BY ISNULL(t.deadline), t.deadline ASC, FIELD(t.priority, '高', '普通', '低'), t.created_at DESC");

    QVariantList params = {userId};
    if (!status.isEmpty()) {
        params.append(status);
    }
    if (courseId > 0) {
        params.append(courseId);
    }
    QSqlQuery query(DbManager::instance().database());
    return readTasks(query, sql, params, errorMessage);
}

QVector<StudyTask> TaskService::tasksDueInDays(int userId, int days, QString *errorMessage) const
{
    QSqlQuery query(DbManager::instance().database());
    const QString sql = QStringLiteral("SELECT t.id, t.user_id, IFNULL(t.course_id, 0), IFNULL(c.name, ''), "
                                       "t.title, t.type, t.content, t.deadline, t.status, t.priority, t.created_at "
                                       "FROM task t LEFT JOIN course c ON t.course_id = c.id "
                                       "WHERE t.user_id = ? AND t.deadline BETWEEN NOW() AND DATE_ADD(NOW(), INTERVAL ? DAY) "
                                       "ORDER BY t.deadline ASC");
    return readTasks(query, sql, {userId, days}, errorMessage);
}

QVector<StudyTask> TaskService::overdueTasks(int userId, QString *errorMessage) const
{
    QSqlQuery query(DbManager::instance().database());
    const QString sql = QStringLiteral("SELECT t.id, t.user_id, IFNULL(t.course_id, 0), IFNULL(c.name, ''), "
                                       "t.title, t.type, t.content, t.deadline, t.status, t.priority, t.created_at "
                                       "FROM task t LEFT JOIN course c ON t.course_id = c.id "
                                       "WHERE t.user_id = ? AND t.status <> '已完成' AND t.deadline < NOW() "
                                       "ORDER BY t.deadline ASC");
    return readTasks(query, sql, {userId}, errorMessage);
}

bool TaskService::addTask(const StudyTask &task, QString *errorMessage)
{
    QSqlQuery query(DbManager::instance().database());
    const QVariant courseId = task.courseId > 0 ? QVariant(task.courseId) : QVariant();
    const QVariantList params = {task.userId, courseId, task.title, task.type, task.content,
                                 task.deadline, task.status, task.priority};
    const QString sql = QStringLiteral("INSERT INTO task (user_id, course_id, title, type, content, deadline, status, priority) "
                                       "VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
    if (!SqlExecutor::exec(query, sql, params, errorMessage)) {
        return false;
    }
    return true;
}

bool TaskService::updateTask(const StudyTask &task, QString *errorMessage)
{
    QSqlQuery query(DbManager::instance().database());
    const QVariant courseId = task.courseId > 0 ? QVariant(task.courseId) : QVariant();
    const QVariantList params = {courseId, task.title, task.type, task.content, task.deadline,
                                 task.status, task.priority, task.id, task.userId};
    const QString sql = QStringLiteral("UPDATE task SET course_id = ?, title = ?, type = ?, content = ?, deadline = ?, status = ?, priority = ? "
                                       "WHERE id = ? AND user_id = ?");
    if (!SqlExecutor::exec(query, sql, params, errorMessage)) {
        return false;
    }
    return true;
}

bool TaskService::removeTask(int taskId, int userId, QString *errorMessage)
{
    QSqlQuery query(DbManager::instance().database());
    if (!SqlExecutor::exec(query,
                           QStringLiteral("DELETE FROM task WHERE id = ? AND user_id = ?"),
                           {taskId, userId},
                           errorMessage)) {
        return false;
    }
    return true;
}

bool TaskService::markCompleted(int taskId, int userId, QString *errorMessage)
{
    QSqlQuery query(DbManager::instance().database());
    if (!SqlExecutor::exec(query,
                           QStringLiteral("UPDATE task SET status = '已完成' WHERE id = ? AND user_id = ?"),
                           {taskId, userId},
                           errorMessage)) {
        return false;
    }
    return true;
}

int TaskService::countTasks(int userId, const QString &status) const
{
    QSqlQuery query(DbManager::instance().database());
    if (status.isEmpty()) {
        SqlExecutor::exec(query, QStringLiteral("SELECT COUNT(*) FROM task WHERE user_id = ?"), {userId});
    } else {
        SqlExecutor::exec(query,
                          QStringLiteral("SELECT COUNT(*) FROM task WHERE user_id = ? AND status = ?"),
                          {userId, status});
    }
    if (query.isActive() && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

int TaskService::overdueCount(int userId) const
{
    QSqlQuery query(DbManager::instance().database());
    if (SqlExecutor::exec(query,
                          QStringLiteral("SELECT COUNT(*) FROM task WHERE user_id = ? AND status <> '已完成' AND deadline < NOW()"),
                          {userId})
        && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

QVector<StudyTask> TaskService::readTasks(QSqlQuery &query,
                                          const QString &sql,
                                          const QVariantList &params,
                                          QString *errorMessage) const
{
    QVector<StudyTask> tasks;
    if (!SqlExecutor::exec(query, sql, params, errorMessage)) {
        return tasks;
    }

    while (query.next()) {
        StudyTask task;
        task.id = query.value(0).toInt();
        task.userId = query.value(1).toInt();
        task.courseId = query.value(2).toInt();
        task.courseName = query.value(3).toString();
        task.title = query.value(4).toString();
        task.type = query.value(5).toString();
        task.content = query.value(6).toString();
        task.deadline = query.value(7).toDateTime();
        task.status = query.value(8).toString();
        task.priority = query.value(9).toString();
        task.createdAt = query.value(10).toDateTime();
        tasks.append(task);
    }
    return tasks;
}
