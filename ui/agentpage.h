#ifndef AGENTPAGE_H
#define AGENTPAGE_H

#include "service/agentservice.h"

#include <QLineEdit>
#include <QTextEdit>
#include <QWidget>

class AgentPage : public QWidget
{
    Q_OBJECT

public:
    explicit AgentPage(int userId, QWidget *parent = nullptr);

private slots:
    void sendMessage();

private:
    int m_userId = 0;
    QTextEdit *m_chatView = nullptr;
    QLineEdit *m_inputEdit = nullptr;
    AgentService *m_agentService = nullptr;
};

#endif // AGENTPAGE_H
