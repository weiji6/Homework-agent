#include "agentservice.h"

#include "service/taskservice.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

AgentService::AgentService(QObject *parent)
    : QObject(parent)
{
}

void AgentService::ask(int userId, const QString &message)
{
    TaskService taskService;
    QString error;
    const Intent intent = detectIntent(message);
    QVector<StudyTask> tasks;

    switch (intent) {
    case Intent::WeeklyTasks:
    case Intent::Plan:
        tasks = taskService.tasksDueInDays(userId, 7, &error);
        break;
    case Intent::Overdue:
        tasks = taskService.overdueTasks(userId, &error);
        break;
    case Intent::Pressure:
    case Intent::General:
    case Intent::CourseTasks:
        tasks = taskService.listTasks(userId, QStringLiteral("未完成"), 0, &error);
        break;
    }

    if (!error.isEmpty()) {
        emit errorOccurred(error);
        return;
    }

    if (intent == Intent::CourseTasks) {
        const QString keyword = extractCourseKeyword(message);
        QVector<StudyTask> filtered;
        for (const StudyTask &task : tasks) {
            if (task.courseName.contains(keyword, Qt::CaseInsensitive) || task.title.contains(keyword, Qt::CaseInsensitive)) {
                filtered.append(task);
            }
        }
        tasks = filtered;
    }

    const QString fallback = localAnswer(intent, tasks, message);
    const QString prompt = QStringLiteral("你是智能学习任务管理系统里的学习助手。请根据数据库任务数据回答用户问题，语气简洁、自然，并给出可执行建议。\n\n用户问题：%1\n\n任务数据：\n%2")
                               .arg(message, buildTaskSummary(tasks));
    callLargeModel(prompt, fallback);
}

AgentService::Intent AgentService::detectIntent(const QString &message) const
{
    if (message.contains(QStringLiteral("逾期")) || message.contains(QStringLiteral("过期"))) {
        return Intent::Overdue;
    }
    if (message.contains(QStringLiteral("计划")) || message.contains(QStringLiteral("安排"))) {
        return Intent::Plan;
    }
    if (message.contains(QStringLiteral("压力")) || message.contains(QStringLiteral("多吗")) || message.contains(QStringLiteral("忙"))) {
        return Intent::Pressure;
    }
    if (message.contains(QStringLiteral("这周")) || message.contains(QStringLiteral("本周")) || message.contains(QStringLiteral("七天"))) {
        return Intent::WeeklyTasks;
    }
    if (message.contains(QStringLiteral("课")) || message.contains(QStringLiteral("课程"))) {
        return Intent::CourseTasks;
    }
    return Intent::General;
}

QString AgentService::extractCourseKeyword(const QString &message) const
{
    QString keyword = message;
    keyword.remove(QStringLiteral("还有什么任务"));
    keyword.remove(QStringLiteral("有哪些任务"));
    keyword.remove(QStringLiteral("课程"));
    keyword.remove(QStringLiteral("课"));
    keyword.remove(QStringLiteral("的"));
    keyword.remove(QStringLiteral("？"));
    keyword.remove(QStringLiteral("?"));
    return keyword.trimmed();
}

QString AgentService::buildTaskSummary(const QVector<StudyTask> &tasks) const
{
    if (tasks.isEmpty()) {
        return QStringLiteral("暂无相关任务。");
    }

    QStringList lines;
    for (int i = 0; i < tasks.size(); ++i) {
        const StudyTask &task = tasks.at(i);
        lines << QStringLiteral("%1. [%2] %3，课程：%4，截止：%5，状态：%6，优先级：%7")
                     .arg(i + 1)
                     .arg(task.type.isEmpty() ? QStringLiteral("任务") : task.type)
                     .arg(task.title)
                     .arg(task.courseName.isEmpty() ? QStringLiteral("未绑定课程") : task.courseName)
                     .arg(task.deadline.isValid() ? task.deadline.toString(QStringLiteral("yyyy-MM-dd HH:mm")) : QStringLiteral("未设置"))
                     .arg(task.status)
                     .arg(task.priority);
    }
    return lines.join(QLatin1Char('\n'));
}

QString AgentService::localAnswer(Intent intent, const QVector<StudyTask> &tasks, const QString &message) const
{
    Q_UNUSED(message)
    if (tasks.isEmpty()) {
        return QStringLiteral("目前没有查到相关未完成任务，可以先复盘一下课程笔记，或者补充新的学习安排。");
    }

    QString intro;
    switch (intent) {
    case Intent::WeeklyTasks:
        intro = QStringLiteral("你这周需要关注 %1 个任务：").arg(tasks.size());
        break;
    case Intent::Pressure:
        intro = QStringLiteral("当前未完成任务有 %1 个，学习压力处于%2。")
                    .arg(tasks.size())
                    .arg(tasks.size() >= 6 ? QStringLiteral("偏高水平") : QStringLiteral("可控范围"));
        break;
    case Intent::Plan:
        intro = QStringLiteral("可以按截止时间这样安排这周学习：");
        break;
    case Intent::Overdue:
        intro = QStringLiteral("你有 %1 个逾期任务，建议优先处理：").arg(tasks.size());
        break;
    case Intent::CourseTasks:
    case Intent::General:
        intro = QStringLiteral("我查到这些相关任务：");
        break;
    }

    QStringList lines;
    lines << intro;
    const int limit = qMin(tasks.size(), 6);
    for (int i = 0; i < limit; ++i) {
        const StudyTask &task = tasks.at(i);
        lines << QStringLiteral("%1. %2（%3，截止 %4，优先级：%5）")
                     .arg(i + 1)
                     .arg(task.title)
                     .arg(task.courseName.isEmpty() ? QStringLiteral("未绑定课程") : task.courseName)
                     .arg(task.deadline.isValid() ? task.deadline.toString(QStringLiteral("MM-dd HH:mm")) : QStringLiteral("未设置"))
                     .arg(task.priority);
    }
    lines << QStringLiteral("建议先做截止时间最近、优先级最高的任务，再处理普通优先级任务。");
    return lines.join(QLatin1Char('\n'));
}

void AgentService::callLargeModel(const QString &prompt, const QString &fallbackAnswer)
{
    const QString apiKey = qEnvironmentVariable("LLM_API_KEY");
    const QString apiUrl = qEnvironmentVariable("LLM_API_URL", "https://api.openai.com/v1/chat/completions");
    const QString model = qEnvironmentVariable("LLM_MODEL", "gpt-4o-mini");

    if (apiKey.isEmpty()) {
        emit answerReady(fallbackAnswer);
        return;
    }

    QNetworkRequest request{QUrl(apiUrl)};
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setRawHeader("Authorization", QByteArray("Bearer ") + apiKey.toUtf8());

    QJsonObject systemMessage;
    systemMessage.insert(QStringLiteral("role"), QStringLiteral("system"));
    systemMessage.insert(QStringLiteral("content"), QStringLiteral("你是一个中文学习助手，只基于提供的数据回答。"));

    QJsonObject userMessage;
    userMessage.insert(QStringLiteral("role"), QStringLiteral("user"));
    userMessage.insert(QStringLiteral("content"), prompt);

    QJsonObject payload;
    payload.insert(QStringLiteral("model"), model);
    payload.insert(QStringLiteral("temperature"), 0.3);
    payload.insert(QStringLiteral("messages"), QJsonArray{systemMessage, userMessage});

    QNetworkReply *reply = m_network.post(request, QJsonDocument(payload).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [this, reply, fallbackAnswer]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit answerReady(fallbackAnswer + QStringLiteral("\n\n提示：API 调用失败，已使用本地分析结果。"));
            return;
        }

        const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        const QJsonArray choices = doc.object().value(QStringLiteral("choices")).toArray();
        if (choices.isEmpty()) {
            emit answerReady(fallbackAnswer);
            return;
        }

        const QString content = choices.first().toObject()
                                    .value(QStringLiteral("message")).toObject()
                                    .value(QStringLiteral("content")).toString();
        emit answerReady(content.isEmpty() ? fallbackAnswer : content.trimmed());
    });
}
