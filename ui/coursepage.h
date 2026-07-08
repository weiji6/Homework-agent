#ifndef COURSEPAGE_H
#define COURSEPAGE_H

#include "model/course.h"
#include "service/courseservice.h"

#include <QTableWidget>
#include <QWidget>

class CoursePage : public QWidget
{
    Q_OBJECT

public:
    explicit CoursePage(int userId, QWidget *parent = nullptr);

private slots:
    void refresh();
    void addCourse();
    void editCourse();
    void deleteCourse();

private:
    Course currentCourse() const;
    bool editDialog(Course *course, const QString &title);

    int m_userId = 0;
    QTableWidget *m_table = nullptr;
    CourseService m_service;
};

#endif // COURSEPAGE_H
