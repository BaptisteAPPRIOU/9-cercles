#include "Postgres.hpp"
#include <QSqlError>
#include <QDebug>

Postgres::Postgres()
    : connectionName_("my_connection")
{
    db_ = QSqlDatabase::addDatabase("QPSQL", connectionName_);
}

Postgres::~Postgres()
{
    if (db_.isOpen()) db_.close();
    QSqlDatabase::removeDatabase(connectionName_);
}

// --- Canonical (Coplien) ---

// Constructeur de copie
Postgres::Postgres(const Postgres& other)
    : connectionName_(other.connectionName_)
{
    db_ = QSqlDatabase::addDatabase("QPSQL", connectionName_);
    // Ne copie PAS l'état de connexion ni les credentials (question de sécurité)
}

// Opérateur d'affectation de copie
Postgres& Postgres::operator=(const Postgres& other)
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
Postgres::Postgres(Postgres&& other) noexcept
    : db_(std::move(other.db_)), connectionName_(std::move(other.connectionName_))
{
    // On laisse other dans un état valide mais vide
}

// Move assignment
Postgres& Postgres::operator=(Postgres&& other) noexcept
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

bool Postgres::connect(const QString& host, int port, const QString& dbName, const QString& user, const QString& password)
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

bool Postgres::isConnected() const
{
    return db_.isOpen();
}

QString Postgres::lastError() const
{
    return db_.lastError().text();
}

QSqlDatabase& Postgres::database()
{
    return db_;
}
