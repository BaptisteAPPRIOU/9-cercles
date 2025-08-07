#include "LPTF_database.hpp"


LPTF_database::LPTF_database()
    : connectionName_("my_connection")
{
    db_ = QSqlDatabase::addDatabase("QPSQL", connectionName_);
}

LPTF_database::~LPTF_database()
{
    if (db_.isOpen()) db_.close();
    QSqlDatabase::removeDatabase(connectionName_);
}

// --- Canonical (Coplien) ---

// Constructeur de copie
LPTF_database::LPTF_database(const LPTF_database& other)
    : connectionName_(other.connectionName_)
{
    db_ = QSqlDatabase::addDatabase("QPSQL", connectionName_);
    // Ne copie PAS l'état de connexion ni les credentials (question de sécurité)
}

// Opérateur d'affectation de copie
LPTF_database& LPTF_database::operator=(const LPTF_database& other)
{
    if (this != &other) {
        if (db_.isOpen()) db_.close();
        QSqlDatabase::removeDatabase(connectionName_);
        connectionName_ = other.connectionName_;
        db_ = QSqlDatabase::addDatabase("QPSQL", connectionName_);
    }
    return *this;
}

// Move constructor
LPTF_database::LPTF_database(LPTF_database&& other) noexcept
    : db_(std::move(other.db_)), connectionName_(std::move(other.connectionName_))
{
    // On laisse other dans un état valide mais vide
}

// Move assignment
LPTF_database& LPTF_database::operator=(LPTF_database&& other) noexcept
{
    if (this != &other) {
        if (db_.isOpen()) db_.close();
        QSqlDatabase::removeDatabase(connectionName_);
        db_ = std::move(other.db_);
        connectionName_ = std::move(other.connectionName_);
    }
    return *this;
}

// ---

bool LPTF_database::connect(const QString& host, int port, const QString& dbName, const QString& user, const QString& password)
{
    db_.setHostName(host);
    db_.setPort(port);
    db_.setDatabaseName(dbName);
    db_.setUserName(user);
    db_.setPassword(password);

    if (!db_.open()) {
        qDebug() << "[Postgres] Erreur de connexion:" << db_.lastError().text();
        return false;
    }
    qDebug() << "[Postgres] Connexion PostgreSQL réussie!";
    return true;
}

bool LPTF_database::isConnected() const
{
    return db_.isOpen();
}

QString LPTF_database::lastError() const
{
    return db_.lastError().text();
}

QSqlDatabase& LPTF_database::database()
{
    return db_;
}

// --- Database operations ---

// --- Client management ---

int LPTF_database::getClientId(const QString& username, const QString& ip)
{
    QSqlQuery query(db_);
    query.prepare("SELECT id FROM client WHERE username = :username AND ip = :ip");
    query.bindValue(":username", username);
    query.bindValue(":ip", ip);
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    // Not found
    return -1;
}


bool LPTF_database::addClient(const QString& username, const QString& ip)
{
    QSqlQuery query(db_);
    query.prepare("INSERT INTO client (username, ip, online_status, last_seen) "
                  "VALUES (:username, :ip, TRUE, NOW()) "
                  "ON CONFLICT (username, ip) DO UPDATE SET online_status = TRUE, last_seen = NOW()");
    query.bindValue(":username", username);
    query.bindValue(":ip", ip);
    if (!query.exec()) {
        qDebug() << "[DB] Erreur insert client:" << query.lastError().text();
        return false;
    }
    return true;
}

// --- New methods for startup data loading ---

QList<QMap<QString, QVariant>> LPTF_database::getAllClients()
{
    QList<QMap<QString, QVariant>> clients;
    QSqlQuery query(db_);
    query.prepare("SELECT id, username, ip, online_status, last_seen FROM client ORDER BY last_seen DESC");
    
    if (!query.exec()) {
        qDebug() << "[DB] Error getting all clients:" << query.lastError().text();
        return clients;
    }
    
    while (query.next()) {
        QMap<QString, QVariant> client;
        client["id"] = query.value("id");
        client["username"] = query.value("username");
        client["ip"] = query.value("ip");
        client["online_status"] = query.value("online_status");
        client["last_seen"] = query.value("last_seen");
        clients.append(client);
    }
    
    qDebug() << "[DB] Retrieved" << clients.size() << "clients from database";
    return clients;
}

bool LPTF_database::updateClientOnlineStatus(const QString& username, const QString& ip, bool isOnline)
{
    QSqlQuery query(db_);
    query.prepare("UPDATE client SET online_status = :status, last_seen = NOW() "
                  "WHERE username = :username AND ip = :ip");
    query.bindValue(":status", isOnline);
    query.bindValue(":username", username);
    query.bindValue(":ip", ip);
    
    if (!query.exec()) {
        qDebug() << "[DB] Error updating client status:" << query.lastError().text();
        return false;
    }
    
    qDebug() << "[DB] Updated client" << username << "(" << ip << ") status to" << (isOnline ? "online" : "offline");
    return true;
}

// --- Command result management ---

bool LPTF_database::addPendingCommand(int clientId, uint16_t packetId, const QString& command)
{
    QSqlQuery query(db_);
    query.prepare("INSERT INTO pending_commands (client_id, packet_id, command, created_at) "
                  "VALUES (:client_id, :packet_id, :command, NOW()) "
                  "ON CONFLICT (client_id, packet_id) DO UPDATE SET command = :command, created_at = NOW()");
    query.bindValue(":client_id", clientId);
    query.bindValue(":packet_id", packetId);
    query.bindValue(":command", command);
    if (!query.exec()) {
        qDebug() << "[DB] Error inserting pending command:" << query.lastError().text();
        return false;
    }
    return true;
}

QString LPTF_database::getPendingCommand(int clientId, uint16_t packetId)
{
    QSqlQuery query(db_);
    query.prepare("SELECT command FROM pending_commands WHERE client_id = :client_id AND packet_id = :packet_id");
    query.bindValue(":client_id", clientId);
    query.bindValue(":packet_id", packetId);
    if (query.exec() && query.next()) {
        return query.value(0).toString();
    }
    return QString(); // Empty string if not found
}

bool LPTF_database::removePendingCommand(int clientId, uint16_t packetId)
{
    QSqlQuery query(db_);
    query.prepare("DELETE FROM pending_commands WHERE client_id = :client_id AND packet_id = :packet_id");
    query.bindValue(":client_id", clientId);
    query.bindValue(":packet_id", packetId);
    if (!query.exec()) {
        qDebug() << "[DB] Error removing pending command:" << query.lastError().text();
        return false;
    }
    return true;
}


bool LPTF_database::addCommandResult(int clientId, const QString& command, const QString& output)
{
    QSqlQuery query(db_);
    query.prepare("INSERT INTO command_result (client_id, command, output) "
                  "VALUES (:client_id, :command, :output)");
    query.bindValue(":client_id", clientId);
    query.bindValue(":command", command);
    query.bindValue(":output", output);
    if (!query.exec()) {
        qDebug() << "[DB] Erreur insert command:" << query.lastError().text();
        return false;
    }
    return true;
}


