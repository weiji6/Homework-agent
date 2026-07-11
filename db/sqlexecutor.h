#ifndef SQLEXECUTOR_H
#define SQLEXECUTOR_H

#include <QString>
#include <QVariant>

class QSqlQuery;

namespace SqlExecutor
{
bool exec(QSqlQuery &query,
          const QString &sql,
          const QVariantList &params = {},
          QString *errorMessage = nullptr);
}

#endif // SQLEXECUTOR_H
