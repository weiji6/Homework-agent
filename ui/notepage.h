#ifndef NOTEPAGE_H
#define NOTEPAGE_H

#include "model/note.h"
#include "service/courseservice.h"
#include "service/noteservice.h"

#include <QComboBox>
#include <QLineEdit>
#include <QTableWidget>
#include <QWidget>

class NotePage : public QWidget
{
    Q_OBJECT

public:
    explicit NotePage(int userId, QWidget *parent = nullptr);

private slots:
    void refresh();
    void addNote();
    void editNote();
    void deleteNote();

private:
    Note currentNote() const;
    bool editDialog(Note *note, const QString &title);

    int m_userId = 0;
    QComboBox *m_courseFilter = nullptr;
    QLineEdit *m_keywordEdit = nullptr;
    QTableWidget *m_table = nullptr;
    NoteService m_noteService;
    CourseService m_courseService;
};

#endif // NOTEPAGE_H
