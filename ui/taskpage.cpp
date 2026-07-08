#include "taskpage.h"

#include <QDateTimeEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

TaskPage::TaskPage(int userId, QWidget *parent)
    : QWidget(parent)
    , m_userId(userId)
{
    auto *layout = new QVBoxLayout(this);
    auto *filters = new QHBoxLayout;
    m_statusFilter = new QComboBox;
    m_statusFilter->addItem(QStringLiteral("全部状态"), QString());
    m_statusFilter->addItem(QStringLiteral("未完成"), QStringLiteral("未完成"));
    m_statusFilter->addItem(QStringLiteral("已完成"), QStringLiteral("已完成"));
    m_courseFilter = new QComboBox;
    auto *refreshButton = new QPushButton(QStringLiteral("刷新"));
    connect(refreshButton, &QPushButton::clicked, this, &TaskPage::refresh);
    connect(m_statusFilter, &QComboBox::currentTextChanged, this, &TaskPage::refresh);
    connect(m_courseFilter, &QComboBox::currentTextChanged, this, &TaskPage::refresh);
    filters->addWidget(m_statusFilter);
    filters->addWidget(m_courseFilter);
    filters->addStretch();
    filters->addWidget(refreshButton);

    auto *actions = new QHBoxLayout;
    auto *addButton = new QPushButton(QStringLiteral("添加任务"));
    auto *editButton = new QPushButton(QStringLiteral("修改任务"));
    auto *doneButton = new QPushButton(QStringLiteral("标记完成"));
    auto *deleteButton = new QPushButton(QStringLiteral("删除任务"));
    connect(addButton, &QPushButton::clicked, this, &TaskPage::addTask);
    connect(editButton, &QPushButton::clicked, this, &TaskPage::editTask);
    connect(doneButton, &QPushButton::clicked, this, &TaskPage::markDone);
    connect(deleteButton, &QPushButton::clicked, this, &TaskPage::deleteTask);
    actions->addWidget(addButton);
    actions->addWidget(editButton);
    actions->addWidget(doneButton);
    actions->addWidget(deleteButton);
    actions->addStretch();

    m_table = new QTableWidget;
    m_table->setColumnCount(9);
    m_table->setHorizontalHeaderLabels({QStringLiteral("ID"), QStringLiteral("标题"), QStringLiteral("课程"), QStringLiteral("类型"), QStringLiteral("截止时间"), QStringLiteral("状态"), QStringLiteral("优先级"), QStringLiteral("内容"), QStringLiteral("课程ID")});
    m_table->setColumnHidden(8, true);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    layout->addLayout(filters);
    layout->addLayout(actions);
    layout->addWidget(m_table);
    refresh();
}

void TaskPage::refresh()
{
    const int oldCourse = m_courseFilter->currentData().toInt();
    m_courseFilter->blockSignals(true);
    m_courseFilter->clear();
    m_courseFilter->addItem(QStringLiteral("全部课程"), 0);
    QString error;
    const QVector<Course> courses = m_courseService.listCourses(m_userId, &error);
    for (const Course &course : courses) {
        m_courseFilter->addItem(course.name, course.id);
    }
    const int courseIndex = m_courseFilter->findData(oldCourse);
    if (courseIndex >= 0) {
        m_courseFilter->setCurrentIndex(courseIndex);
    }
    m_courseFilter->blockSignals(false);

    const QVector<StudyTask> tasks = m_taskService.listTasks(m_userId,
                                                             m_statusFilter->currentData().toString(),
                                                             m_courseFilter->currentData().toInt(),
                                                             &error);
    if (!error.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("查询失败"), error);
        return;
    }

    m_table->setRowCount(tasks.size());
    for (int row = 0; row < tasks.size(); ++row) {
        const StudyTask &task = tasks.at(row);
        const QStringList values = {
            QString::number(task.id),
            task.title,
            task.courseName,
            task.type,
            task.deadline.toString(QStringLiteral("yyyy-MM-dd HH:mm")),
            task.status,
            task.priority,
            task.content,
            QString::number(task.courseId)
        };
        for (int col = 0; col < values.size(); ++col) {
            m_table->setItem(row, col, new QTableWidgetItem(values.at(col)));
        }
    }
}

void TaskPage::addTask()
{
    StudyTask task;
    task.userId = m_userId;
    task.deadline = QDateTime::currentDateTime().addDays(1);
    if (!editDialog(&task, QStringLiteral("添加任务"))) {
        return;
    }
    QString error;
    if (!m_taskService.addTask(task, &error)) {
        QMessageBox::warning(this, QStringLiteral("保存失败"), error);
        return;
    }
    refresh();
}

void TaskPage::editTask()
{
    StudyTask task = currentTask();
    if (task.id == 0) {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请先选择一个任务。"));
        return;
    }
    if (!editDialog(&task, QStringLiteral("修改任务"))) {
        return;
    }
    QString error;
    if (!m_taskService.updateTask(task, &error)) {
        QMessageBox::warning(this, QStringLiteral("保存失败"), error);
        return;
    }
    refresh();
}

void TaskPage::deleteTask()
{
    const StudyTask task = currentTask();
    if (task.id == 0) {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请先选择一个任务。"));
        return;
    }
    if (QMessageBox::question(this, QStringLiteral("确认删除"), QStringLiteral("确定删除任务“%1”吗？").arg(task.title)) != QMessageBox::Yes) {
        return;
    }
    QString error;
    if (!m_taskService.removeTask(task.id, m_userId, &error)) {
        QMessageBox::warning(this, QStringLiteral("删除失败"), error);
        return;
    }
    refresh();
}

void TaskPage::markDone()
{
    const StudyTask task = currentTask();
    if (task.id == 0) {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请先选择一个任务。"));
        return;
    }
    QString error;
    if (!m_taskService.markCompleted(task.id, m_userId, &error)) {
        QMessageBox::warning(this, QStringLiteral("更新失败"), error);
        return;
    }
    refresh();
}

StudyTask TaskPage::currentTask() const
{
    StudyTask task;
    task.userId = m_userId;
    const int row = m_table->currentRow();
    if (row < 0) {
        return task;
    }
    task.id = m_table->item(row, 0)->text().toInt();
    task.title = m_table->item(row, 1)->text();
    task.courseName = m_table->item(row, 2)->text();
    task.type = m_table->item(row, 3)->text();
    task.deadline = QDateTime::fromString(m_table->item(row, 4)->text(), QStringLiteral("yyyy-MM-dd HH:mm"));
    task.status = m_table->item(row, 5)->text();
    task.priority = m_table->item(row, 6)->text();
    task.content = m_table->item(row, 7)->text();
    task.courseId = m_table->item(row, 8)->text().toInt();
    return task;
}

bool TaskPage::editDialog(StudyTask *task, const QString &title)
{
    QDialog dialog(this);
    dialog.setWindowTitle(title);
    auto *form = new QFormLayout(&dialog);
    auto *titleEdit = new QLineEdit(task->title);
    auto *courseBox = new QComboBox;
    courseBox->addItem(QStringLiteral("不绑定课程"), 0);
    const QVector<Course> courses = m_courseService.listCourses(m_userId);
    for (const Course &course : courses) {
        courseBox->addItem(course.name, course.id);
    }
    const int courseIndex = courseBox->findData(task->courseId);
    if (courseIndex >= 0) {
        courseBox->setCurrentIndex(courseIndex);
    }
    auto *typeBox = new QComboBox;
    typeBox->addItems({QStringLiteral("作业"), QStringLiteral("考试"), QStringLiteral("复习"), QStringLiteral("实验"), QStringLiteral("阅读"), QStringLiteral("其他")});
    typeBox->setCurrentText(task->type.isEmpty() ? QStringLiteral("作业") : task->type);
    auto *deadlineEdit = new QDateTimeEdit(task->deadline.isValid() ? task->deadline : QDateTime::currentDateTime().addDays(1));
    deadlineEdit->setCalendarPopup(true);
    deadlineEdit->setDisplayFormat(QStringLiteral("yyyy-MM-dd HH:mm"));
    auto *statusBox = new QComboBox;
    statusBox->addItems({QStringLiteral("未完成"), QStringLiteral("已完成")});
    statusBox->setCurrentText(task->status);
    auto *priorityBox = new QComboBox;
    priorityBox->addItems({QStringLiteral("高"), QStringLiteral("普通"), QStringLiteral("低")});
    priorityBox->setCurrentText(task->priority);
    auto *contentEdit = new QTextEdit(task->content);
    contentEdit->setMinimumHeight(120);
    form->addRow(QStringLiteral("标题"), titleEdit);
    form->addRow(QStringLiteral("课程"), courseBox);
    form->addRow(QStringLiteral("类型"), typeBox);
    form->addRow(QStringLiteral("截止时间"), deadlineEdit);
    form->addRow(QStringLiteral("状态"), statusBox);
    form->addRow(QStringLiteral("优先级"), priorityBox);
    form->addRow(QStringLiteral("内容"), contentEdit);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    form->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) {
        return false;
    }
    task->title = titleEdit->text().trimmed();
    task->courseId = courseBox->currentData().toInt();
    task->type = typeBox->currentText();
    task->deadline = deadlineEdit->dateTime();
    task->status = statusBox->currentText();
    task->priority = priorityBox->currentText();
    task->content = contentEdit->toPlainText().trimmed();
    return !task->title.isEmpty();
}
