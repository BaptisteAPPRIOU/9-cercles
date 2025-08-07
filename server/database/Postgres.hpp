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
class Postgres
{
public:
    Postgres();
    ~Postgres();

    // Canonical form (Coplien)
    Postgres(const Postgres& other);
    Postgres& operator=(const Postgres& other);

    Postgres(Postgres&& other) noexcept;
    Postgres& operator=(Postgres&& other) noexcept;

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
