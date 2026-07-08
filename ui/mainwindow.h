#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(int userId, const QString &username, QWidget *parent = nullptr);

signals:
    void loggedOut();

private:
    void addMenuButton(const QString &text, int pageIndex);

    int m_userId = 0;
    QString m_username;
    QStackedWidget *m_stack = nullptr;
};

#endif // MAINWINDOW_H
