#include "agentpage.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>

AgentPage::AgentPage(int userId, QWidget *parent)
    : QWidget(parent)
    , m_userId(userId)
    , m_agentService(new AgentService(this))
{
    auto *layout = new QVBoxLayout(this);
    m_chatView = new QTextEdit;
    m_chatView->setReadOnly(true);
    m_chatView->setPlaceholderText(QStringLiteral("可以问：这周有哪些作业没完成？数据库课还有什么任务？帮我总结一下最近的学习压力。"));
    m_inputEdit = new QLineEdit;
    m_inputEdit->setPlaceholderText(QStringLiteral("输入你的问题"));
    auto *sendButton = new QPushButton(QStringLiteral("发送"));
    auto *bottom = new QHBoxLayout;
    bottom->addWidget(m_inputEdit, 1);
    bottom->addWidget(sendButton);
    layout->addWidget(m_chatView, 1);
    layout->addLayout(bottom);

    connect(sendButton, &QPushButton::clicked, this, &AgentPage::sendMessage);
    connect(m_inputEdit, &QLineEdit::returnPressed, this, &AgentPage::sendMessage);
    connect(m_agentService, &AgentService::answerReady, this, [this](const QString &answer) {
        m_chatView->append(QStringLiteral("<b>AI 助手：</b><br>%1<br>").arg(answer.toHtmlEscaped().replace(QLatin1Char('\n'), QStringLiteral("<br>"))));
    });
    connect(m_agentService, &AgentService::errorOccurred, this, [this](const QString &message) {
        m_chatView->append(QStringLiteral("<b>系统：</b>%1").arg(message.toHtmlEscaped()));
    });
}

void AgentPage::sendMessage()
{
    const QString message = m_inputEdit->text().trimmed();
    if (message.isEmpty()) {
        return;
    }
    m_inputEdit->clear();
    m_chatView->append(QStringLiteral("<b>我：</b>%1").arg(message.toHtmlEscaped()));
    m_agentService->ask(m_userId, message);
}
