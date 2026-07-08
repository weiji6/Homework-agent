#include "noteservice.h"

#include "db/dbmanager.h"

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
    query.prepare(sql);
    query.addBindValue(userId);
    if (courseId > 0) {
        query.addBindValue(courseId);
    }
    if (!keyword.trimmed().isEmpty()) {
        const QString like = QStringLiteral("%%1%").arg(keyword.trimmed());
        query.addBindValue(like);
        query.addBindValue(like);
    }

    if (!query.exec()) {
        if (errorMessage) {
            *errorMessage = query.lastError().text();
        }
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
    query.prepare(QStringLiteral("INSERT INTO note (user_id, course_id, title, content) VALUES (?, ?, ?, ?)"));
    query.addBindValue(note.userId);
    if (note.courseId > 0) {
        query.addBindValue(note.courseId);
    } else {
        query.addBindValue(QVariant());
    }
    query.addBindValue(note.title);
    query.addBindValue(note.content);
    if (!query.exec()) {
        if (errorMessage) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }
    return true;
}

bool NoteService::updateNote(const Note &note, QString *errorMessage)
{
    QSqlQuery query(DbManager::instance().database());
    query.prepare(QStringLiteral("UPDATE note SET course_id = ?, title = ?, content = ? WHERE id = ? AND user_id = ?"));
    if (note.courseId > 0) {
        query.addBindValue(note.courseId);
    } else {
        query.addBindValue(QVariant());
    }
    query.addBindValue(note.title);
    query.addBindValue(note.content);
    query.addBindValue(note.id);
    query.addBindValue(note.userId);
    if (!query.exec()) {
        if (errorMessage) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }
    return true;
}

bool NoteService::removeNote(int noteId, int userId, QString *errorMessage)
{
    QSqlQuery query(DbManager::instance().database());
    query.prepare(QStringLiteral("DELETE FROM note WHERE id = ? AND user_id = ?"));
    query.addBindValue(noteId);
    query.addBindValue(userId);
    if (!query.exec()) {
        if (errorMessage) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }
    return true;
}
