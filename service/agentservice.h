#ifndef AGENTSERVICE_H
#define AGENTSERVICE_H

#include "model/task.h"

#include <QNetworkAccessManager>
#include <QObject>
#include <QString>
#include <QVector>

class AgentService : public QObject
{
    Q_OBJECT

public:
    explicit AgentService(QObject *parent = nullptr);
    void ask(int userId, const QString &message);

signals:
    void answerReady(const QString &answer);
    void answerStarted();
    void answerChunkReady(const QString &chunk);
    void answerFinished();
    void errorOccurred(const QString &message);

private:
    enum class Intent {
        WeeklyTasks,
        CourseTasks,
        Pressure,
        Plan,
        Overdue,
        Knowledge,
        General
    };

    Intent detectIntent(const QString &message) const;
    QString extractCourseKeyword(const QString &message) const;
    QString buildTaskSummary(const QVector<StudyTask> &tasks) const;
    QString localAnswer(Intent intent, const QVector<StudyTask> &tasks, const QString &message) const;
    QString knowledgeFallback(const QString &message) const;
    QString normalizeChatCompletionsUrl(const QString &apiUrl) const;
    void callLargeModel(const QString &prompt, const QString &fallbackAnswer);
    void callLargeModelStream(const QString &prompt, const QString &fallbackAnswer);

    QNetworkAccessManager m_network;
};

#endif // AGENTSERVICE_H
