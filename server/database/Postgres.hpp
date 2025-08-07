#pragma once

#include <QString>
#include <QSqlDatabase>

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

private:
    QSqlDatabase db_;
    QString connectionName_;
};
