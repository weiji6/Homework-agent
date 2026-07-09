#ifndef ENVLOADER_H
#define ENVLOADER_H

#include <QMap>
#include <QString>
#include <QStringList>

class EnvLoader
{
public:
    static QStringList candidatePaths();
    static bool load(QMap<QString, QString> *values,
                     QString *filePath = nullptr,
                     QString *errorMessage = nullptr);
    static QString value(const QMap<QString, QString> &values,
                         const QString &key,
                         const QString &fallback = QString());

private:
    static bool parseLine(const QString &line, QString *key, QString *value);
    static QString unquote(QString value);
};

#endif // ENVLOADER_H
