#ifndef BASEDAO_H
#define BASEDAO_H

#include <QObject>
#include <QSqlQuery>
#include <QSqlError>


class BaseDAO : public QObject
{
    Q_OBJECT
public:
    explicit BaseDAO(QObject *parent = nullptr);
    void handleError(const QString functionName, const QSqlQuery &query) const;
    const QSqlDatabase &database() const { return m_db; }

protected:
    QSqlDatabase m_db;
    QString lastError;

// signals:
//     void databaseError(const QSqlError &error) const;
};

#endif // BASEDAO_H
