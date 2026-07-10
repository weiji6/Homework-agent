#include "agentservice.h"

#include "config/envloader.h"
#include "service/taskservice.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

namespace {

QString extractContentFromChatCompletion(const QByteArray &body)
{
    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        return {};
    }

    const QJsonArray choices = doc.object().value(QStringLiteral("choices")).toArray();
    if (choices.isEmpty()) {
        return {};
    }

    return choices.first().toObject()
        .value(QStringLiteral("message")).toObject()
        .value(QStringLiteral("content")).toString()
        .trimmed();
}

QString extractDeltaFromStreamData(const QByteArray &data)
{
    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        return {};
    }

    const QJsonArray choices = doc.object().value(QStringLiteral("choices")).toArray();
    if (choices.isEmpty()) {
        return {};
    }

    const QJsonObject choice = choices.first().toObject();
    const QString deltaContent = choice.value(QStringLiteral("delta")).toObject()
                                     .value(QStringLiteral("content")).toString();
    if (!deltaContent.isEmpty()) {
        return deltaContent;
    }

    return choice.value(QStringLiteral("message")).toObject()
        .value(QStringLiteral("content")).toString();
}

template<typename DeltaHandler>
void processStreamBuffer(QByteArray *buffer, bool flushRemainder, DeltaHandler onDelta)
{
    while (true) {
        const int newlineIndex = buffer->indexOf('\n');
        if (newlineIndex < 0) {
            break;
        }

        QByteArray line = buffer->left(newlineIndex).trimmed();
        buffer->remove(0, newlineIndex + 1);
        if (line.isEmpty() || line.startsWith(':') || !line.startsWith("data:")) {
            continue;
        }

        const QByteArray data = line.mid(5).trimmed();
        if (data == "[DONE]") {
            continue;
        }

        const QString delta = extractDeltaFromStreamData(data);
        if (!delta.isEmpty()) {
            onDelta(delta);
        }
    }

    if (!flushRemainder || buffer->isEmpty()) {
        return;
    }

    QByteArray line = buffer->trimmed();
    buffer->clear();
    if (!line.startsWith("data:")) {
        return;
    }

    const QByteArray data = line.mid(5).trimmed();
    if (data == "[DONE]") {
        return;
    }

    const QString delta = extractDeltaFromStreamData(data);
    if (!delta.isEmpty()) {
        onDelta(delta);
    }
}

} // namespace

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

    if (intent == Intent::Knowledge) {
        const QString prompt = QStringLiteral("你是一个耐心的中文学习助手。用户正在学习，请直接讲解问题，不要查询任务数据库。\n\n用户问题：%1")
                                   .arg(message);
        callLargeModelStream(prompt, knowledgeFallback(message));
        return;
    }

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
    case Intent::Knowledge:
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
            if (task.courseName.contains(keyword, Qt::CaseInsensitive) ||
                task.title.contains(keyword, Qt::CaseInsensitive)) {
                filtered.append(task);
            }
        }
        tasks = filtered;
    }

    const QString fallback = localAnswer(intent, tasks, message);
    const QString prompt = QStringLiteral("你是智能学习任务管理系统里的学习助手。请根据数据库任务数据回答用户问题，语气简洁自然，并给出可执行建议。\n\n用户问题：%1\n\n任务数据：\n%2")
                               .arg(message, buildTaskSummary(tasks));
    callLargeModelStream(prompt, fallback);
}

AgentService::Intent AgentService::detectIntent(const QString &message) const
{
    const QString text = message.trimmed();

    const QStringList knowledgeWords = {
        QStringLiteral("教我"),
        QStringLiteral("怎么"),
        QStringLiteral("如何"),
        QStringLiteral("什么是"),
        QStringLiteral("语法"),
        QStringLiteral("使用方法"),
        QStringLiteral("讲一下"),
        QStringLiteral("解释"),
        QStringLiteral("学习")
    };
    const QStringList taskWords = {
        QStringLiteral("任务"),
        QStringLiteral("作业"),
        QStringLiteral("考试"),
        QStringLiteral("截止"),
        QStringLiteral("完成"),
        QStringLiteral("逾期"),
        QStringLiteral("这周"),
        QStringLiteral("本周")
    };

    bool looksLikeKnowledge = false;
    for (const QString &word : knowledgeWords) {
        if (text.contains(word)) {
            looksLikeKnowledge = true;
            break;
        }
    }

    bool looksLikeTask = false;
    for (const QString &word : taskWords) {
        if (text.contains(word)) {
            looksLikeTask = true;
            break;
        }
    }

    if (looksLikeKnowledge && !looksLikeTask) {
        return Intent::Knowledge;
    }
    if (text.contains(QStringLiteral("逾期")) || text.contains(QStringLiteral("过期"))) {
        return Intent::Overdue;
    }
    if (text.contains(QStringLiteral("计划")) || text.contains(QStringLiteral("安排"))) {
        return Intent::Plan;
    }
    if (text.contains(QStringLiteral("压力")) || text.contains(QStringLiteral("多吗")) || text.contains(QStringLiteral("忙"))) {
        return Intent::Pressure;
    }
    if (text.contains(QStringLiteral("这周")) || text.contains(QStringLiteral("本周")) || text.contains(QStringLiteral("七天"))) {
        return Intent::WeeklyTasks;
    }
    if ((text.contains(QStringLiteral("课")) || text.contains(QStringLiteral("课程"))) && looksLikeTask) {
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
        return QStringLiteral("目前没有查到相关未完成任务。");
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
    case Intent::Knowledge:
        return knowledgeFallback(message);
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

QString AgentService::knowledgeFallback(const QString &message) const
{
    if (message.contains(QStringLiteral("MySQL"), Qt::CaseInsensitive) ||
        message.contains(QStringLiteral("查询")) ||
        message.contains(QStringLiteral("SQL"), Qt::CaseInsensitive)) {
        return QStringLiteral("MySQL 查询语句最常用的是 SELECT：\n"
                              "1. 查询所有列：SELECT * FROM 表名;\n"
                              "2. 查询指定列：SELECT 列1, 列2 FROM 表名;\n"
                              "3. 条件查询：SELECT * FROM 表名 WHERE 条件;\n"
                              "4. 排序：SELECT * FROM 表名 ORDER BY 列名 ASC 或 DESC;\n"
                              "5. 限制数量：SELECT * FROM 表名 LIMIT 10;\n"
                              "6. 分组统计：SELECT 字段, COUNT(*) FROM 表名 GROUP BY 字段;\n\n"
                              "例子：SELECT title, deadline FROM task WHERE status = '未完成' ORDER BY deadline ASC;\n"
                              "这句会查询未完成任务的标题和截止时间，并按截止时间升序排列。");
    }

    return QStringLiteral("这个问题更适合由大模型回答，但当前 API 没有成功返回。请检查 LLM_API_KEY、LLM_API_URL 和 LLM_MODEL。");
}

QString AgentService::normalizeChatCompletionsUrl(const QString &apiUrl) const
{
    QString url = apiUrl.trimmed();
    while (url.endsWith(QLatin1Char('/'))) {
        url.chop(1);
    }
    if (url.endsWith(QStringLiteral("/chat/completions"))) {
        return url;
    }
    return url + QStringLiteral("/chat/completions");
}

void AgentService::callLargeModel(const QString &prompt, const QString &fallbackAnswer)
{
    QMap<QString, QString> values;
    EnvLoader::load(&values);

    const QString apiKey = EnvLoader::value(values, QStringLiteral("LLM_API_KEY"));
    const QString apiUrl = normalizeChatCompletionsUrl(
        EnvLoader::value(values, QStringLiteral("LLM_API_URL"), QStringLiteral("https://api.openai.com/v1/chat/completions")));
    const QString model = EnvLoader::value(values, QStringLiteral("LLM_MODEL"), QStringLiteral("gpt-4o-mini"));
    const double temperature = EnvLoader::value(values, QStringLiteral("LLM_TEMPERATURE"), QStringLiteral("0.3")).toDouble();

    if (apiKey.isEmpty()) {
        emit answerReady(fallbackAnswer + QStringLiteral("\n\n提示：未配置 LLM_API_KEY，已使用本地回答。"));
        return;
    }

    QNetworkRequest request{QUrl(apiUrl)};
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setRawHeader("Authorization", QByteArray("Bearer ") + apiKey.toUtf8());

    QJsonObject systemMessage;
    systemMessage.insert(QStringLiteral("role"), QStringLiteral("system"));
    systemMessage.insert(QStringLiteral("content"), QStringLiteral("你是一个中文学习助手，回答要清楚、简洁、适合初学者。"));

    QJsonObject userMessage;
    userMessage.insert(QStringLiteral("role"), QStringLiteral("user"));
    userMessage.insert(QStringLiteral("content"), prompt);

    QJsonObject payload;
    payload.insert(QStringLiteral("model"), model);
    payload.insert(QStringLiteral("temperature"), temperature);
    payload.insert(QStringLiteral("messages"), QJsonArray{systemMessage, userMessage});

    QNetworkReply *reply = m_network.post(request, QJsonDocument(payload).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [this, reply, fallbackAnswer]() {
        const QByteArray body = reply->readAll();
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            emit answerReady(fallbackAnswer + QStringLiteral("\n\n提示：API 调用失败：%1\n响应：%2")
                                             .arg(reply->errorString(), QString::fromUtf8(body).left(500)));
            return;
        }

        const QJsonDocument doc = QJsonDocument::fromJson(body);
        const QJsonArray choices = doc.object().value(QStringLiteral("choices")).toArray();
        if (choices.isEmpty()) {
            emit answerReady(fallbackAnswer + QStringLiteral("\n\n提示：API 没有返回 choices。响应：%1")
                                             .arg(QString::fromUtf8(body).left(500)));
            return;
        }

        const QString content = choices.first().toObject()
                                    .value(QStringLiteral("message")).toObject()
                                    .value(QStringLiteral("content")).toString();
        emit answerReady(content.isEmpty() ? fallbackAnswer : content.trimmed());
    });
}

void AgentService::callLargeModelStream(const QString &prompt, const QString &fallbackAnswer)
{
    QMap<QString, QString> values;
    EnvLoader::load(&values);

    const QString apiKey = EnvLoader::value(values, QStringLiteral("LLM_API_KEY"));
    const QString apiUrl = normalizeChatCompletionsUrl(
        EnvLoader::value(values, QStringLiteral("LLM_API_URL"), QStringLiteral("https://api.openai.com/v1/chat/completions")));
    const QString model = EnvLoader::value(values, QStringLiteral("LLM_MODEL"), QStringLiteral("gpt-4o-mini"));
    const double temperature = EnvLoader::value(values, QStringLiteral("LLM_TEMPERATURE"), QStringLiteral("0.3")).toDouble();

    emit answerStarted();

    if (apiKey.isEmpty()) {
        emit answerChunkReady(fallbackAnswer + QStringLiteral("\n\n提示：未配置 LLM_API_KEY，已使用本地回答。"));
        emit answerFinished();
        return;
    }

    QNetworkRequest request{QUrl(apiUrl)};
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setRawHeader("Accept", "text/event-stream");
    request.setRawHeader("Authorization", QByteArray("Bearer ") + apiKey.toUtf8());

    QJsonObject systemMessage;
    systemMessage.insert(QStringLiteral("role"), QStringLiteral("system"));
    systemMessage.insert(QStringLiteral("content"), QStringLiteral("You are a Chinese study assistant. Answer clearly, concisely, and in a beginner-friendly way."));

    QJsonObject userMessage;
    userMessage.insert(QStringLiteral("role"), QStringLiteral("user"));
    userMessage.insert(QStringLiteral("content"), prompt);

    QJsonObject payload;
    payload.insert(QStringLiteral("model"), model);
    payload.insert(QStringLiteral("temperature"), temperature);
    payload.insert(QStringLiteral("stream"), true);
    payload.insert(QStringLiteral("messages"), QJsonArray{systemMessage, userMessage});

    QNetworkReply *reply = m_network.post(request, QJsonDocument(payload).toJson(QJsonDocument::Compact));
    auto *streamBuffer = new QByteArray;
    auto *rawBody = new QByteArray;
    auto *answer = new QString;

    connect(reply, &QNetworkReply::readyRead, this, [this, reply, streamBuffer, rawBody, answer]() {
        const QByteArray chunk = reply->readAll();
        rawBody->append(chunk);
        streamBuffer->append(chunk);
        processStreamBuffer(streamBuffer, false, [this, answer](const QString &delta) {
            answer->append(delta);
            emit answerChunkReady(delta);
        });
    });

    connect(reply, &QNetworkReply::finished, this, [this, reply, streamBuffer, rawBody, answer, fallbackAnswer]() {
        const QByteArray remaining = reply->readAll();
        if (!remaining.isEmpty()) {
            rawBody->append(remaining);
            streamBuffer->append(remaining);
        }

        processStreamBuffer(streamBuffer, true, [this, answer](const QString &delta) {
            answer->append(delta);
            emit answerChunkReady(delta);
        });

        const QNetworkReply::NetworkError networkError = reply->error();
        const QString errorString = reply->errorString();

        if (networkError != QNetworkReply::NoError) {
            const QString message = answer->isEmpty()
                ? fallbackAnswer + QStringLiteral("\n\n提示：API 调用失败：%1\n响应：%2")
                      .arg(errorString, QString::fromUtf8(*rawBody).left(500))
                : QStringLiteral("\n\n提示：API 调用中断：%1").arg(errorString);
            emit answerChunkReady(message);
        } else if (answer->isEmpty()) {
            const QString content = extractContentFromChatCompletion(*rawBody);
            if (content.isEmpty()) {
                emit answerChunkReady(fallbackAnswer + QStringLiteral("\n\n提示：API 没有返回可解析的回答。响应：%1")
                                                       .arg(QString::fromUtf8(*rawBody).left(500)));
            } else {
                emit answerChunkReady(content);
            }
        }

        emit answerFinished();

        delete streamBuffer;
        delete rawBody;
        delete answer;
        reply->deleteLater();
    });
}
