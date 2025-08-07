#pragma once

#include <QString>
#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>
#include <QSqlQuery>
#include <QVariant>
#include <QList>
#include <QMap>

/**
 * @brief Gère la connexion à une base de données PostgreSQL via Qt.
 */
class LPTF_database
{
public:
    LPTF_database();
    ~LPTF_database();

    // Canonical form (Coplien)
    LPTF_database(const LPTF_database& other);
    LPTF_database& operator=(const LPTF_database& other);

    LPTF_database(LPTF_database&& other) noexcept;
    LPTF_database& operator=(LPTF_database&& other) noexcept;

    bool connect(const QString& host, int port, const QString& dbName, const QString& user, const QString& password);
    bool isConnected() const;
    QString lastError() const;

    QSqlDatabase& database();

    int getClientId(const QString& username, const QString& ip);

    bool addClient(const QString& username, const QString& ip);

    // New methods for loading client data at startup
    QList<QMap<QString, QVariant>> getAllClients();
    bool updateClientOnlineStatus(const QString& username, const QString& ip, bool isOnline);

    bool addPendingCommand(int clientId, uint16_t packetId, const QString& command);
    
    QString getPendingCommand(int clientId, uint16_t packetId);
    
    bool removePendingCommand(int clientId, uint16_t packetId);

    bool addCommandResult(int clientId, const QString& command, const QString& output);

private:
    QSqlDatabase db_;
    QString connectionName_;
};
