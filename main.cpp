#include "ui/loginwindow.h"

#include <QApplication>
#include <QFont>

namespace {

void applyAppStyle(QApplication &app)
{
    QFont font(QStringLiteral("Microsoft YaHei UI"), 10);
    app.setFont(font);

    app.setStyleSheet(QStringLiteral(R"(
        QWidget {
            color: #23302f;
            background: #f4f7f5;
            font-family: "Microsoft YaHei UI", "Microsoft YaHei", "Segoe UI";
            font-size: 14px;
        }

        QLabel {
            background: transparent;
        }

        QWidget#LoginWindow {
            background: #eef5f1;
        }

        QFrame#LoginCard,
        QFrame#Sidebar,
        QGroupBox {
            border-radius: 12px;
        }

        QFrame#LoginCard {
            background: #ffffff;
            border: 1px solid #dfe9e4;
        }

        QFrame#Sidebar {
            background: #173c37;
            border-radius: 0;
        }

        QLabel#AppTitle {
            color: #163c37;
            font-size: 24px;
            font-weight: 700;
            background: transparent;
        }

        QLabel#PageTitle {
            color: #1d3f3a;
            font-size: 22px;
            font-weight: 700;
            background: transparent;
        }

        QLabel#MutedText {
            color: #6f7d79;
            background: transparent;
        }

        QLabel#SectionTitle {
            color: #24433f;
            font-size: 15px;
            font-weight: 700;
            background: transparent;
        }

        QLabel#SidebarTitle,
        QLabel#SidebarUser {
            color: #f5fbf8;
            background: transparent;
        }

        QLabel#SidebarTitle {
            font-size: 18px;
            font-weight: 700;
        }

        QLabel#SidebarUser {
            color: #b9d8cf;
        }

        QPushButton {
            min-height: 34px;
            padding: 7px 16px;
            border: 1px solid #cfe0d8;
            border-radius: 8px;
            background: #ffffff;
            color: #25413d;
            font-weight: 600;
        }

        QPushButton:hover {
            background: #edf7f2;
            border-color: #9bc8ba;
        }

        QPushButton:pressed {
            background: #d9eee5;
        }

        QPushButton[primary="true"] {
            background: #1f8a70;
            border-color: #1f8a70;
            color: #ffffff;
        }

        QPushButton[primary="true"]:hover {
            background: #19765f;
            border-color: #19765f;
        }

        QPushButton[danger="true"] {
            color: #a13b36;
            border-color: #edc7c3;
            background: #fff8f7;
        }

        QPushButton[danger="true"]:hover {
            background: #ffece9;
        }

        QFrame#Sidebar QPushButton {
            min-height: 38px;
            padding: 8px 14px;
            text-align: left;
            border: 0;
            border-radius: 9px;
            background: transparent;
            color: #e8f5ef;
        }

        QFrame#Sidebar QPushButton:hover {
            background: rgba(255, 255, 255, 0.10);
        }

        QFrame#Sidebar QPushButton:checked {
            background: #e8f5ef;
            color: #173c37;
        }

        QLineEdit,
        QTextEdit,
        QComboBox,
        QDateTimeEdit {
            min-height: 32px;
            padding: 6px 10px;
            border: 1px solid #d5e1dc;
            border-radius: 6px;
            background-color: #ffffff;
            selection-background-color: #99d5c5;
        }

        QLineEdit:focus,
        QTextEdit:focus,
        QComboBox:focus,
        QDateTimeEdit:focus {
            border: 1px solid #1f8a70;
        }

        QTextEdit {
            line-height: 1.35;
        }

        QTextEdit,
        QTextEdit viewport {
            background-color: #ffffff;
        }

        QComboBox {
            padding-right: 34px;
        }

        QComboBox::drop-down,
        QDateTimeEdit::drop-down {
            subcontrol-origin: padding;
            subcontrol-position: top right;
            width: 30px;
            border: 0;
            border-left: 1px solid #e5eee9;
            border-top-right-radius: 6px;
            border-bottom-right-radius: 6px;
            background-color: #f6faf8;
        }

        QComboBox::down-arrow,
        QDateTimeEdit::down-arrow {
            image: none;
            width: 0;
            height: 0;
            border-left: 5px solid transparent;
            border-right: 5px solid transparent;
            border-top: 6px solid #52706a;
            margin-right: 9px;
        }

        QComboBox QAbstractItemView {
            padding: 4px;
            border: 1px solid #cfe0d8;
            border-radius: 6px;
            background: #ffffff;
            selection-background-color: #d7eee6;
            selection-color: #1c3733;
            outline: 0;
        }

        QTableWidget {
            gridline-color: #e6eee9;
            border: 1px solid #dce8e2;
            border-radius: 6px;
            background-color: #ffffff;
            alternate-background-color: #f7fbf8;
            selection-background-color: #d7eee6;
            selection-color: #1c3733;
        }

        QTableWidget viewport {
            background-color: #ffffff;
            border-radius: 6px;
        }

        QTableCornerButton::section {
            background: #eaf4ef;
            border: 0;
        }

        QHeaderView::section {
            min-height: 34px;
            padding: 7px 10px;
            border: 0;
            border-bottom: 1px solid #dce8e2;
            background: #eaf4ef;
            color: #22413d;
            font-weight: 700;
        }

        QGroupBox {
            margin-top: 18px;
            padding: 18px 14px 14px 14px;
            border: 1px solid #dce8e2;
            border-radius: 8px;
            background-color: #ffffff;
            font-weight: 700;
            color: #21413b;
        }

        QGroupBox::title {
            subcontrol-origin: margin;
            left: 12px;
            padding: 0 6px;
            background: #ffffff;
        }

        QScrollBar:vertical {
            width: 10px;
            background: transparent;
        }

        QScrollBar::handle:vertical {
            min-height: 40px;
            border-radius: 5px;
            background: #bfd5cd;
        }

        QScrollBar::add-line:vertical,
        QScrollBar::sub-line:vertical {
            height: 0;
        }
    )"));
}

} // namespace

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QApplication::setApplicationName(QStringLiteral("智能学习任务管理系统"));
    QApplication::setOrganizationName(QStringLiteral("HomeworkAgent"));
    applyAppStyle(a);

    LoginWindow w;
    w.show();
    return QApplication::exec();
}
