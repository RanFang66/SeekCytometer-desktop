#include "BaseDAO.h"
#include "DatabaseManager.h"

BaseDAO::BaseDAO(QObject *parent)
    : QObject{parent}, m_db{DatabaseManager::getDatabase()}
{}

void BaseDAO::handleError(const QString functionName, const QSqlQuery &query) const
{
    const QSqlError error = query.lastError();
    if (error.isValid()) {
        QString lastError = QString("Database error occured in %1: %2").arg(functionName, error.text().toUtf8());
        qWarning() << lastError;

        emit databaseError(error);
    }
}
