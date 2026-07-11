#include "noteservice.h"

#include "db/dbmanager.h"
#include "db/sqlexecutor.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

NoteService::NoteService(QObject *parent)
    : QObject(parent)
{
}

QVector<Note> NoteService::listNotes(int userId, int courseId, const QString &keyword, QString *errorMessage) const
{
    QVector<Note> notes;
    QString sql = QStringLiteral("SELECT n.id, n.user_id, IFNULL(n.course_id, 0), IFNULL(c.name, ''), "
                                 "n.title, n.content, n.created_at, n.updated_at "
                                 "FROM note n LEFT JOIN course c ON n.course_id = c.id WHERE n.user_id = ?");
    if (courseId > 0) {
        sql += QStringLiteral(" AND n.course_id = ?");
    }
    if (!keyword.trimmed().isEmpty()) {
        sql += QStringLiteral(" AND (n.title LIKE ? OR n.content LIKE ?)");
    }
    sql += QStringLiteral(" ORDER BY n.updated_at DESC");

    QSqlQuery query(DbManager::instance().database());
    QVariantList params = {userId};
    if (courseId > 0) {
        params.append(courseId);
    }
    if (!keyword.trimmed().isEmpty()) {
        const QString like = QStringLiteral("%%1%").arg(keyword.trimmed());
        params.append(like);
        params.append(like);
    }

    if (!SqlExecutor::exec(query, sql, params, errorMessage)) {
        return notes;
    }

    while (query.next()) {
        Note note;
        note.id = query.value(0).toInt();
        note.userId = query.value(1).toInt();
        note.courseId = query.value(2).toInt();
        note.courseName = query.value(3).toString();
        note.title = query.value(4).toString();
        note.content = query.value(5).toString();
        note.createdAt = query.value(6).toDateTime();
        note.updatedAt = query.value(7).toDateTime();
        notes.append(note);
    }
    return notes;
}

bool NoteService::addNote(const Note &note, QString *errorMessage)
{
    QSqlQuery query(DbManager::instance().database());
    const QVariant courseId = note.courseId > 0 ? QVariant(note.courseId) : QVariant();
    const QVariantList params = {note.userId, courseId, note.title, note.content};
    if (!SqlExecutor::exec(query,
                           QStringLiteral("INSERT INTO note (user_id, course_id, title, content) VALUES (?, ?, ?, ?)"),
                           params,
                           errorMessage)) {
        return false;
    }
    return true;
}

bool NoteService::updateNote(const Note &note, QString *errorMessage)
{
    QSqlQuery query(DbManager::instance().database());
    const QVariant courseId = note.courseId > 0 ? QVariant(note.courseId) : QVariant();
    const QVariantList params = {courseId, note.title, note.content, note.id, note.userId};
    if (!SqlExecutor::exec(query,
                           QStringLiteral("UPDATE note SET course_id = ?, title = ?, content = ? WHERE id = ? AND user_id = ?"),
                           params,
                           errorMessage)) {
        return false;
    }
    return true;
}

bool NoteService::removeNote(int noteId, int userId, QString *errorMessage)
{
    QSqlQuery query(DbManager::instance().database());
    if (!SqlExecutor::exec(query,
                           QStringLiteral("DELETE FROM note WHERE id = ? AND user_id = ?"),
                           {noteId, userId},
                           errorMessage)) {
        return false;
    }
    return true;
}
