#include "sqlexecutor.h"

#include "dbmanager.h"

#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>

namespace
{
QString formatValue(const QSqlDatabase &database, const QVariant &value)
{
    if (!value.isValid() || value.isNull()) {
        return QStringLiteral("NULL");
    }

    QSqlField field(QString(), value.metaType());
    field.setValue(value);
    return database.driver()->formatValue(field);
}

bool buildOdbcStatement(const QSqlDatabase &database,
                        const QString &sql,
                        const QVariantList &params,
                        QString *statement,
                        QString *errorMessage)
{
    QString result;
    result.reserve(sql.size() + params.size() * 8);

    qsizetype paramIndex = 0;
    bool inString = false;
    for (qsizetype i = 0; i < sql.size(); ++i) {
        const QChar character = sql.at(i);
        if (character == QLatin1Char('\'')) {
            result += character;
            if (inString && i + 1 < sql.size() && sql.at(i + 1) == QLatin1Char('\'')) {
                result += sql.at(++i);
            } else {
                inString = !inString;
            }
            continue;
        }

        if (character == QLatin1Char('?') && !inString) {
            if (paramIndex >= params.size()) {
                if (errorMessage) {
                    *errorMessage = QStringLiteral("SQL 参数数量不足");
                }
                return false;
            }
            result += formatValue(database, params.at(paramIndex++));
        } else {
            result += character;
        }
    }

    if (paramIndex != params.size()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("SQL 参数数量不匹配");
        }
        return false;
    }

    *statement = result;
    return true;
}
}

bool SqlExecutor::exec(QSqlQuery &query,
                       const QString &sql,
                       const QVariantList &params,
                       QString *errorMessage)
{
    const QSqlDatabase database = DbManager::instance().database();
    bool success = false;

    if (database.driverName() == QStringLiteral("QODBC")) {
        QString statement;
        if (!buildOdbcStatement(database, sql, params, &statement, errorMessage)) {
            return false;
        }
        success = query.exec(statement);
    } else {
        success = query.prepare(sql);
        if (success) {
            for (const QVariant &param : params) {
                query.addBindValue(param);
            }
            success = query.exec();
        }
    }

    if (!success && errorMessage) {
        *errorMessage = query.lastError().text();
    }
    return success;
}
