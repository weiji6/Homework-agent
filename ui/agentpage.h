#ifndef AGENTPAGE_H
#define AGENTPAGE_H

#include "service/agentservice.h"

#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include <QString>
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
    void setComposerEnabled(bool enabled);
    void refreshChatView(bool includeStreamingAnswer = false);
    void scheduleStreamingRefresh();

    int m_userId = 0;
    QTextEdit *m_chatView = nullptr;
    QLineEdit *m_inputEdit = nullptr;
    QPushButton *m_sendButton = nullptr;
    QTimer *m_streamRefreshTimer = nullptr;
    AgentService *m_agentService = nullptr;
    QString m_chatHtml;
    QString m_streamingAnswer;
    bool m_waitingForAnswer = false;
};

#endif // AGENTPAGE_H
