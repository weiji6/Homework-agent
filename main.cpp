#include "ui/loginwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QApplication::setApplicationName("智能学习任务管理系统");
    QApplication::setOrganizationName("HomeworkAgent");

    LoginWindow w;
    w.show();
    return QApplication::exec();
}
