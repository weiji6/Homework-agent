#include "notepage.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

NotePage::NotePage(int userId, QWidget *parent)
    : QWidget(parent)
    , m_userId(userId)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(28, 24, 28, 24);
    layout->setSpacing(14);
    auto *filters = new QHBoxLayout;
    filters->setSpacing(10);
    m_courseFilter = new QComboBox;
    m_keywordEdit = new QLineEdit;
    m_keywordEdit->setPlaceholderText(QStringLiteral("搜索标题或内容"));
    auto *searchButton = new QPushButton(QStringLiteral("查询"));
    connect(searchButton, &QPushButton::clicked, this, &NotePage::refresh);
    filters->addWidget(m_courseFilter);
    filters->addWidget(m_keywordEdit);
    filters->addWidget(searchButton);

    auto *actions = new QHBoxLayout;
    actions->setSpacing(10);
    auto *addButton = new QPushButton(QStringLiteral("添加笔记"));
    auto *editButton = new QPushButton(QStringLiteral("修改笔记"));
    auto *deleteButton = new QPushButton(QStringLiteral("删除笔记"));
    addButton->setProperty("primary", true);
    deleteButton->setProperty("danger", true);
    connect(addButton, &QPushButton::clicked, this, &NotePage::addNote);
    connect(editButton, &QPushButton::clicked, this, &NotePage::editNote);
    connect(deleteButton, &QPushButton::clicked, this, &NotePage::deleteNote);
    actions->addWidget(addButton);
    actions->addWidget(editButton);
    actions->addWidget(deleteButton);
    actions->addStretch();

    m_table = new QTableWidget;
    m_table->setColumnCount(7);
    m_table->setHorizontalHeaderLabels({QStringLiteral("ID"), QStringLiteral("标题"), QStringLiteral("课程"), QStringLiteral("内容"), QStringLiteral("更新时间"), QStringLiteral("课程ID"), QStringLiteral("创建时间")});
    m_table->setColumnHidden(5, true);
    m_table->setColumnHidden(6, true);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->verticalHeader()->setVisible(false);
    m_table->verticalHeader()->setDefaultSectionSize(38);
    m_table->setAlternatingRowColors(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    layout->addLayout(filters);
    layout->addLayout(actions);
    layout->addWidget(m_table);
    refresh();
}

void NotePage::refresh()
{
    const int oldCourse = m_courseFilter->currentData().toInt();
    m_courseFilter->blockSignals(true);
    m_courseFilter->clear();
    m_courseFilter->addItem(QStringLiteral("全部课程"), 0);
    const QVector<Course> courses = m_courseService.listCourses(m_userId);
    for (const Course &course : courses) {
        m_courseFilter->addItem(course.name, course.id);
    }
    const int courseIndex = m_courseFilter->findData(oldCourse);
    if (courseIndex >= 0) {
        m_courseFilter->setCurrentIndex(courseIndex);
    }
    m_courseFilter->blockSignals(false);

    QString error;
    const QVector<Note> notes = m_noteService.listNotes(m_userId,
                                                        m_courseFilter->currentData().toInt(),
                                                        m_keywordEdit->text(),
                                                        &error);
    if (!error.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("查询失败"), error);
        return;
    }

    m_table->setRowCount(notes.size());
    for (int row = 0; row < notes.size(); ++row) {
        const Note &note = notes.at(row);
        const QStringList values = {
            QString::number(note.id),
            note.title,
            note.courseName,
            note.content.left(120),
            note.updatedAt.toString(QStringLiteral("yyyy-MM-dd HH:mm")),
            QString::number(note.courseId),
            note.createdAt.toString(QStringLiteral("yyyy-MM-dd HH:mm"))
        };
        for (int col = 0; col < values.size(); ++col) {
            m_table->setItem(row, col, new QTableWidgetItem(values.at(col)));
        }
        m_table->item(row, 3)->setData(Qt::UserRole, note.content);
    }
}

void NotePage::addNote()
{
    Note note;
    note.userId = m_userId;
    if (!editDialog(&note, QStringLiteral("添加笔记"))) {
        return;
    }
    QString error;
    if (!m_noteService.addNote(note, &error)) {
        QMessageBox::warning(this, QStringLiteral("保存失败"), error);
        return;
    }
    refresh();
}

void NotePage::editNote()
{
    Note note = currentNote();
    if (note.id == 0) {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请先选择一条笔记。"));
        return;
    }
    if (!editDialog(&note, QStringLiteral("修改笔记"))) {
        return;
    }
    QString error;
    if (!m_noteService.updateNote(note, &error)) {
        QMessageBox::warning(this, QStringLiteral("保存失败"), error);
        return;
    }
    refresh();
}

void NotePage::deleteNote()
{
    const Note note = currentNote();
    if (note.id == 0) {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请先选择一条笔记。"));
        return;
    }
    if (QMessageBox::question(this, QStringLiteral("确认删除"), QStringLiteral("确定删除笔记“%1”吗？").arg(note.title)) != QMessageBox::Yes) {
        return;
    }
    QString error;
    if (!m_noteService.removeNote(note.id, m_userId, &error)) {
        QMessageBox::warning(this, QStringLiteral("删除失败"), error);
        return;
    }
    refresh();
}

Note NotePage::currentNote() const
{
    Note note;
    note.userId = m_userId;
    const int row = m_table->currentRow();
    if (row < 0) {
        return note;
    }
    note.id = m_table->item(row, 0)->text().toInt();
    note.title = m_table->item(row, 1)->text();
    note.courseName = m_table->item(row, 2)->text();
    note.content = m_table->item(row, 3)->data(Qt::UserRole).toString();
    note.courseId = m_table->item(row, 5)->text().toInt();
    return note;
}

bool NotePage::editDialog(Note *note, const QString &title)
{
    QDialog dialog(this);
    dialog.setWindowTitle(title);
    auto *form = new QFormLayout(&dialog);
    auto *titleEdit = new QLineEdit(note->title);
    auto *courseBox = new QComboBox;
    courseBox->addItem(QStringLiteral("不绑定课程"), 0);
    const QVector<Course> courses = m_courseService.listCourses(m_userId);
    for (const Course &course : courses) {
        courseBox->addItem(course.name, course.id);
    }
    const int courseIndex = courseBox->findData(note->courseId);
    if (courseIndex >= 0) {
        courseBox->setCurrentIndex(courseIndex);
    }
    auto *contentEdit = new QTextEdit(note->content);
    contentEdit->setMinimumHeight(260);
    form->addRow(QStringLiteral("标题"), titleEdit);
    form->addRow(QStringLiteral("课程"), courseBox);
    form->addRow(QStringLiteral("内容"), contentEdit);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    form->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) {
        return false;
    }
    note->title = titleEdit->text().trimmed();
    note->courseId = courseBox->currentData().toInt();
    note->content = contentEdit->toPlainText().trimmed();
    return !note->title.isEmpty();
}
