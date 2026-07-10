#include "coursepage.h"

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

CoursePage::CoursePage(int userId, QWidget *parent)
    : QWidget(parent)
    , m_userId(userId)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(28, 24, 28, 24);
    layout->setSpacing(14);
    auto *actions = new QHBoxLayout;
    actions->setSpacing(10);
    auto *addButton = new QPushButton(QStringLiteral("添加课程"));
    auto *editButton = new QPushButton(QStringLiteral("修改课程"));
    auto *deleteButton = new QPushButton(QStringLiteral("删除课程"));
    auto *refreshButton = new QPushButton(QStringLiteral("刷新"));
    addButton->setProperty("primary", true);
    deleteButton->setProperty("danger", true);
    connect(addButton, &QPushButton::clicked, this, &CoursePage::addCourse);
    connect(editButton, &QPushButton::clicked, this, &CoursePage::editCourse);
    connect(deleteButton, &QPushButton::clicked, this, &CoursePage::deleteCourse);
    connect(refreshButton, &QPushButton::clicked, this, &CoursePage::refresh);
    actions->addWidget(addButton);
    actions->addWidget(editButton);
    actions->addWidget(deleteButton);
    actions->addStretch();
    actions->addWidget(refreshButton);

    m_table = new QTableWidget;
    m_table->setColumnCount(7);
    m_table->setHorizontalHeaderLabels({QStringLiteral("ID"), QStringLiteral("课程名"), QStringLiteral("教师"), QStringLiteral("教室"), QStringLiteral("星期"), QStringLiteral("开始"), QStringLiteral("结束")});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->verticalHeader()->setVisible(false);
    m_table->verticalHeader()->setDefaultSectionSize(38);
    m_table->setAlternatingRowColors(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    layout->addLayout(actions);
    layout->addWidget(m_table);
    refresh();
}

void CoursePage::refresh()
{
    QString error;
    const QVector<Course> courses = m_service.listCourses(m_userId, &error);
    if (!error.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("查询失败"), error);
        return;
    }

    m_table->setRowCount(courses.size());
    for (int row = 0; row < courses.size(); ++row) {
        const Course &course = courses.at(row);
        const QStringList values = {
            QString::number(course.id),
            course.name,
            course.teacher,
            course.classroom,
            QString::number(course.weekday),
            course.startTime,
            course.endTime
        };
        for (int col = 0; col < values.size(); ++col) {
            m_table->setItem(row, col, new QTableWidgetItem(values.at(col)));
        }
    }
}

void CoursePage::addCourse()
{
    Course course;
    course.userId = m_userId;
    if (!editDialog(&course, QStringLiteral("添加课程"))) {
        return;
    }

    QString error;
    if (!m_service.addCourse(course, &error)) {
        QMessageBox::warning(this, QStringLiteral("保存失败"), error);
        return;
    }
    refresh();
}

void CoursePage::editCourse()
{
    Course course = currentCourse();
    if (course.id == 0) {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请先选择一门课程。"));
        return;
    }
    if (!editDialog(&course, QStringLiteral("修改课程"))) {
        return;
    }

    QString error;
    if (!m_service.updateCourse(course, &error)) {
        QMessageBox::warning(this, QStringLiteral("保存失败"), error);
        return;
    }
    refresh();
}

void CoursePage::deleteCourse()
{
    const Course course = currentCourse();
    if (course.id == 0) {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请先选择一门课程。"));
        return;
    }
    if (QMessageBox::question(this, QStringLiteral("确认删除"), QStringLiteral("确定删除课程“%1”吗？").arg(course.name)) != QMessageBox::Yes) {
        return;
    }

    QString error;
    if (!m_service.removeCourse(course.id, m_userId, &error)) {
        QMessageBox::warning(this, QStringLiteral("删除失败"), error);
        return;
    }
    refresh();
}

Course CoursePage::currentCourse() const
{
    Course course;
    course.userId = m_userId;
    const int row = m_table->currentRow();
    if (row < 0) {
        return course;
    }
    course.id = m_table->item(row, 0)->text().toInt();
    course.name = m_table->item(row, 1)->text();
    course.teacher = m_table->item(row, 2)->text();
    course.classroom = m_table->item(row, 3)->text();
    course.weekday = m_table->item(row, 4)->text().toInt();
    course.startTime = m_table->item(row, 5)->text();
    course.endTime = m_table->item(row, 6)->text();
    return course;
}

bool CoursePage::editDialog(Course *course, const QString &title)
{
    QDialog dialog(this);
    dialog.setWindowTitle(title);
    auto *form = new QFormLayout(&dialog);
    auto *nameEdit = new QLineEdit(course->name);
    auto *teacherEdit = new QLineEdit(course->teacher);
    auto *classroomEdit = new QLineEdit(course->classroom);
    auto *weekdayBox = new QComboBox;
    for (int i = 1; i <= 7; ++i) {
        weekdayBox->addItem(QStringLiteral("星期%1").arg(i), i);
    }
    weekdayBox->setCurrentIndex(qMax(0, course->weekday - 1));
    auto *startEdit = new QLineEdit(course->startTime);
    auto *endEdit = new QLineEdit(course->endTime);
    form->addRow(QStringLiteral("课程名"), nameEdit);
    form->addRow(QStringLiteral("教师"), teacherEdit);
    form->addRow(QStringLiteral("教室"), classroomEdit);
    form->addRow(QStringLiteral("星期"), weekdayBox);
    form->addRow(QStringLiteral("开始时间"), startEdit);
    form->addRow(QStringLiteral("结束时间"), endEdit);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    form->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) {
        return false;
    }
    course->name = nameEdit->text().trimmed();
    course->teacher = teacherEdit->text().trimmed();
    course->classroom = classroomEdit->text().trimmed();
    course->weekday = weekdayBox->currentData().toInt();
    course->startTime = startEdit->text().trimmed();
    course->endTime = endEdit->text().trimmed();
    return !course->name.isEmpty();
}
