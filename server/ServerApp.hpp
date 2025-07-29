#ifndef SERVERAPP_HPP
#define SERVERAPP_HPP

#include "../utils/EnvLoader.hpp"
#include "../utils/LPTF/LPTF_Socket.hpp"
#include "../utils/LPTF/LPTF_Packet.hpp"

#include <vector>
#include <memory>
#include <string>
#include <iostream>
#include <cstring>
#include <QObject>

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
    // charge .env, init Winsock, bind & listen
    explicit ServerApp(const std::string &envFilePath);
    ~ServerApp();

    void run();

signals:
    void clientConnected(const QString &clientInfo);

private:
    LPTF_Socket m_serverSocket;
    std::vector<std::unique_ptr<LPTF_Socket>> m_clients;
    std::string m_envFilePath;
};

#endif // SERVERAPP_HPP
