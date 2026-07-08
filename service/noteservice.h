#ifndef NOTESERVICE_H
#define NOTESERVICE_H

#include "model/note.h"

#include <QObject>
#include <QVector>

class NoteService : public QObject
{
    Q_OBJECT

public:
    explicit NoteService(QObject *parent = nullptr);

    QVector<Note> listNotes(int userId, int courseId = 0, const QString &keyword = QString(), QString *errorMessage = nullptr) const;
    bool addNote(const Note &note, QString *errorMessage = nullptr);
    bool updateNote(const Note &note, QString *errorMessage = nullptr);
    bool removeNote(int noteId, int userId, QString *errorMessage = nullptr);
};

#endif // NOTESERVICE_H
