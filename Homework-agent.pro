QT += widgets sql network

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    config/envloader.cpp \
    db/dbconfig.cpp \
    db/dbmanager.cpp \
    db/sqlexecutor.cpp \
    main.cpp \
    service/agentservice.cpp \
    service/courseservice.cpp \
    service/noteservice.cpp \
    service/taskservice.cpp \
    service/userservice.cpp \
    ui/agentpage.cpp \
    ui/coursepage.cpp \
    ui/dashboardpage.cpp \
    ui/loginwindow.cpp \
    ui/mainwindow.cpp \
    ui/notepage.cpp \
    ui/statspage.cpp \
    ui/taskpage.cpp

HEADERS += \
    config/envloader.h \
    db/dbconfig.h \
    db/dbmanager.h \
    db/sqlexecutor.h \
    model/course.h \
    model/note.h \
    model/task.h \
    model/user.h \
    service/agentservice.h \
    service/courseservice.h \
    service/noteservice.h \
    service/taskservice.h \
    service/userservice.h \
    ui/agentpage.h \
    ui/coursepage.h \
    ui/dashboardpage.h \
    ui/loginwindow.h \
    ui/mainwindow.h \
    ui/notepage.h \
    ui/statspage.h \
    ui/taskpage.h

DISTFILES += \
    .env \
    .env.example \
    README.md \
    sql/init.sql

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
