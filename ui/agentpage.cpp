#include "agentpage.h"

#include <QHBoxLayout>
#include <QRegularExpression>
#include <QPushButton>
#include <QTextCursor>
#include <QTextDocument>
#include <QVBoxLayout>

namespace {

constexpr int StreamingRefreshIntervalMs = 60;

QString extractBodyHtml(const QString &html)
{
    const QRegularExpression bodyPattern(QStringLiteral(R"(<body[^>]*>(.*)</body>)"),
                                         QRegularExpression::DotMatchesEverythingOption);
    const QRegularExpressionMatch match = bodyPattern.match(html);
    return match.hasMatch() ? match.captured(1).trimmed() : html;
}

QString renderMessageText(const QString &text)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QTextDocument document;
    document.setDefaultStyleSheet(QStringLiteral(
        "body { font-family: 'Microsoft YaHei', 'Segoe UI', sans-serif; font-size: 10pt; }"
        "h1 { font-size: 18pt; margin: 10px 0 6px; }"
        "h2 { font-size: 15pt; margin: 9px 0 5px; }"
        "h3 { font-size: 12pt; margin: 8px 0 4px; }"
        "p { margin: 4px 0; }"
        "ul, ol { margin-top: 4px; margin-bottom: 4px; }"
        "pre { background: #f4f6f8; padding: 8px; border-radius: 4px; }"
        "code { background: #f4f6f8; padding: 1px 3px; border-radius: 3px; }"));
    document.setMarkdown(text);
    return extractBodyHtml(document.toHtml());
#else
    QString html = text.toHtmlEscaped();

    const QRegularExpression boldPattern(QStringLiteral(R"(\*\*(.+?)\*\*)"),
                                         QRegularExpression::DotMatchesEverythingOption);
    html.replace(boldPattern, QStringLiteral("<strong>\\1</strong>"));

    html.replace(QStringLiteral("\r\n"), QStringLiteral("<br>"));
    html.replace(QLatin1Char('\n'), QStringLiteral("<br>"));
    html.replace(QLatin1Char('\r'), QStringLiteral("<br>"));
    return html;
#endif
}

QString messageBlock(const QString &speaker, const QString &message)
{
    return QStringLiteral("<div style=\"margin:10px 0 14px 0;\">"
                          "<div style=\"font-weight:600; margin-bottom:4px;\">%1</div>"
                          "<div>%2</div>"
                          "</div>")
        .arg(speaker.toHtmlEscaped(), renderMessageText(message));
}

QString chatDocumentHtml(const QString &body)
{
    return QStringLiteral("<html><head><style>"
                          "body { font-family: 'Microsoft YaHei', 'Segoe UI', sans-serif; font-size: 10pt; line-height: 1.45; }"
                          "h1 { font-size: 18pt; margin: 10px 0 6px; }"
                          "h2 { font-size: 15pt; margin: 9px 0 5px; }"
                          "h3 { font-size: 12pt; margin: 8px 0 4px; }"
                          "p { margin: 4px 0; }"
                          "ul, ol { margin-top: 4px; margin-bottom: 4px; }"
                          "pre { background: #f4f6f8; padding: 8px; border-radius: 4px; }"
                          "code { background: #f4f6f8; padding: 1px 3px; border-radius: 3px; }"
                          "</style></head><body>%1</body></html>")
        .arg(body);
}

} // namespace

AgentPage::AgentPage(int userId, QWidget *parent)
    : QWidget(parent)
    , m_userId(userId)
    , m_agentService(new AgentService(this))
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(28, 24, 28, 24);
    layout->setSpacing(14);
    m_chatView = new QTextEdit;
    m_chatView->setReadOnly(true);
    m_chatView->setPlaceholderText(QStringLiteral("可以问：这周有哪些作业没完成？数据库课还有什么任务？帮我总结一下最近的学习压力。"));

    m_inputEdit = new QLineEdit;
    m_inputEdit->setPlaceholderText(QStringLiteral("输入你的问题"));

    m_sendButton = new QPushButton(QStringLiteral("发送"));
    m_streamRefreshTimer = new QTimer(this);
    m_streamRefreshTimer->setSingleShot(true);
    auto *bottom = new QHBoxLayout;
    bottom->setSpacing(10);
    bottom->addWidget(m_inputEdit, 1);
    bottom->addWidget(m_sendButton);
    layout->addWidget(m_chatView, 1);
    layout->addLayout(bottom);

    connect(m_sendButton, &QPushButton::clicked, this, &AgentPage::sendMessage);
    connect(m_inputEdit, &QLineEdit::returnPressed, this, &AgentPage::sendMessage);
    connect(m_streamRefreshTimer, &QTimer::timeout, this, [this]() {
        refreshChatView(true);
    });

    connect(m_agentService, &AgentService::answerStarted, this, [this]() {
        m_streamingAnswer.clear();
        refreshChatView(true);
    });
    connect(m_agentService, &AgentService::answerChunkReady, this, [this](const QString &chunk) {
        m_streamingAnswer += chunk;
        scheduleStreamingRefresh();
    });
    connect(m_agentService, &AgentService::answerFinished, this, [this]() {
        m_streamRefreshTimer->stop();
        m_chatHtml += messageBlock(QStringLiteral("AI 助手："), m_streamingAnswer);
        m_streamingAnswer.clear();
        refreshChatView();
        m_waitingForAnswer = false;
        setComposerEnabled(true);
    });
    connect(m_agentService, &AgentService::answerReady, this, [this](const QString &answer) {
        m_chatHtml += messageBlock(QStringLiteral("AI 助手："), answer);
        refreshChatView();
        m_waitingForAnswer = false;
        setComposerEnabled(true);
    });
    connect(m_agentService, &AgentService::errorOccurred, this, [this](const QString &message) {
        m_chatHtml += messageBlock(QStringLiteral("系统："), message);
        refreshChatView();
        m_waitingForAnswer = false;
        setComposerEnabled(true);
    });
}

void AgentPage::sendMessage()
{
    if (m_waitingForAnswer) {
        return;
    }

    const QString message = m_inputEdit->text().trimmed();
    if (message.isEmpty()) {
        return;
    }

    m_inputEdit->clear();
    m_chatHtml += messageBlock(QStringLiteral("我："), message);
    refreshChatView();
    m_waitingForAnswer = true;
    setComposerEnabled(false);
    m_agentService->ask(m_userId, message);
}

void AgentPage::setComposerEnabled(bool enabled)
{
    m_inputEdit->setEnabled(enabled);
    if (m_sendButton) {
        m_sendButton->setEnabled(enabled);
    }
    if (enabled) {
        m_inputEdit->setFocus();
    }
}

void AgentPage::refreshChatView(bool includeStreamingAnswer)
{
    QString html = m_chatHtml;
    if (includeStreamingAnswer) {
        html += messageBlock(QStringLiteral("AI 助手："), m_streamingAnswer);
    }

    m_chatView->setHtml(chatDocumentHtml(html));
    m_chatView->moveCursor(QTextCursor::End);
}

void AgentPage::scheduleStreamingRefresh()
{
    if (!m_streamRefreshTimer->isActive()) {
        m_streamRefreshTimer->start(StreamingRefreshIntervalMs);
    }
}
