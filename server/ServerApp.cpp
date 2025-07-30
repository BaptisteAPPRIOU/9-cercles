#include <iostream>
#include "ServerApp.hpp"

ServerApp::ServerApp(const std::string &envFilePath)
    : m_envFilePath(envFilePath)
{
    // UTF-8 console on Windows
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    // Load .env
    EnvLoader::loadEnv(m_envFilePath);
    int port = EnvLoader::loadPort(m_envFilePath);
    std::cout << "ServerApp: écoute sur le port " << port << "...\n";

    // Bind & listen
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
        // Prepare fd_set
        fd_set readfds;
        FD_ZERO(&readfds);

        int listenFd = m_serverSocket.getSocketFd();
        FD_SET(listenFd, &readfds);
        int maxFd = listenFd;

        for (const auto &c : m_clients)
        {
            int fd = c->getSocketFd();
            FD_SET(fd, &readfds);
            if (fd > maxFd)
                maxFd = fd;
        }

        // Blocking until activity
        int activity = select(maxFd + 1, &readfds, nullptr, nullptr, nullptr);
        if (activity < 0)
        {
            std::cerr << "select() error: " << std::strerror(errno) << "\n";
            break;
        }

        // New connection: store socket and placeholder for username
        if (FD_ISSET(listenFd, &readfds))
        {
            auto newCl = m_serverSocket.acceptSocket();
            std::cout << "Client connecté: " << newCl->getClientIP() << " (awaiting user info)\n";
            m_clients.push_back(std::move(newCl));
            m_clientUsers.push_back(std::string());
        }

        // Read from existing clients
        for (size_t idx = 0; idx < m_clients.size(); )
        {
            auto &client = m_clients[idx];
            int fd = client->getSocketFd();
            if (FD_ISSET(fd, &readfds))
            {
                try
                {
                    auto data = client->recvBinary();
                    auto pkt = LPTF_Packet::deserialize(data);
                    std::string msg(pkt.getPayload().begin(), pkt.getPayload().end());
                    // Handle initial GET_INFO to extract actual username
                    if (pkt.getType() == PacketType::GET_INFO)
                    {
                        const std::string key = "\"username\":\"";
                        std::string username;
                        auto pos = msg.find(key);
                        if (pos != std::string::npos)
                        {
                            pos += key.length();
                            auto end = msg.find('"', pos);
                            if (end != std::string::npos)
                                username = msg.substr(pos, end - pos);
                        }
                        m_clientUsers[idx] = username;
                        std::string ip = client->getClientIP();
                        std::string clientInfo = username + " " + ip;
                        std::cout << "Client info received: " << clientInfo << "\n";
                        emit clientConnected(QString::fromStdString(clientInfo));
                        ++idx;
                        continue;
                    }
                    std::cout << "Reçu de " << client->getClientIP() << " : " << msg << "\n";
                    // Echo back
                    std::string resp = "ACK: " + msg;
                    std::vector<uint8_t> pay(resp.begin(), resp.end());
                    LPTF_Packet ack(1, PacketType::RESPONSE, 0, 0, 0, pay);
                    client->sendBinary(ack.serialize());
                    ++idx;
                }
                catch (const std::exception &e)
                {
                    std::cout << "Déconnexion de " << client->getClientIP()
                              << " (" << e.what() << ")\n";
                    m_clients.erase(m_clients.begin() + idx);
                    m_clientUsers.erase(m_clientUsers.begin() + idx);
                    // do not increment idx, next element shifts into current idx
                }
            }
            else
            {
                ++idx;
            }
        }
    }
}
void ServerApp::debugSelectionButton() {
    std::cout << "button clicked" << std::endl;
}

void ServerApp::onGetInfoSys(const QString& clientId) {
    std::cout << "[SERVER DEBUG] onGetInfoSys called for " << clientId.toStdString() << std::endl;
    // Find the client socket by IP (clientId)
    for (const auto& c : m_clients) {
        if (QString::fromStdString(c->getClientIP()) == clientId) {
            std::cout << "[SERVER DEBUG] Found client socket for " << clientId.toStdString() << std::endl;
            // Send GET_INFO request with empty payload
            LPTF_Packet request(1, PacketType::GET_INFO, 0, 1, 1, {});
            c->sendBinary(request.serialize());
            std::cout << "[SERVER DEBUG] Sent GET_INFO packet to client " << clientId.toStdString() << std::endl;
            // Receive response from client
            try {
                auto data = c->recvBinary();
                auto packet = LPTF_Packet::deserialize(data);
                std::string payload(packet.getPayload().begin(), packet.getPayload().end());
                std::cout << "[SERVER DEBUG] Received response from client " << clientId.toStdString() << ": " << payload << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "[SERVER DEBUG] Error receiving or deserializing response from client " << clientId.toStdString() << ": " << e.what() << std::endl;
            }
            return;
        }
    }
    std::cout << "[SERVER DEBUG] No client found for " << clientId.toStdString() << std::endl;
    return;
}