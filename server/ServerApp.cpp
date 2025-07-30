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

        // New connection?
        if (FD_ISSET(listenFd, &readfds))
        {
            auto newCl = m_serverSocket.acceptSocket();
            std::string ip = newCl->getClientIP();
            std::cout << "Client connecté: " << ip << "\n";
            m_clients.push_back(std::move(newCl));
            // Emit signal to notify UI
            emit clientConnected(QString::fromStdString(ip));
        }

        // Read from existing clients
        for (auto it = m_clients.begin(); it != m_clients.end();)
        {
            int fd = (*it)->getSocketFd();
            if (FD_ISSET(fd, &readfds))
            {
                try
                {
                    auto data = (*it)->recvBinary();
                    auto pkt = LPTF_Packet::deserialize(data);
                    std::string msg(pkt.getPayload().begin(), pkt.getPayload().end());
                    std::cout << "Reçu de " << (*it)->getClientIP() << " : " << msg << "\n";

                    // Echo back
                    std::string resp = "ACK: " + msg;
                    std::vector<uint8_t> pay(resp.begin(), resp.end());
                    LPTF_Packet ack(1, PacketType::RESPONSE, 0, 0, 0, pay);
                    (*it)->sendBinary(ack.serialize());

                    ++it;
                }
                catch (const std::exception &e)
                {
                    std::cout << "Déconnexion de " << (*it)->getClientIP()
                              << " (" << e.what() << ")\n";
                    it = m_clients.erase(it);
                }
            }
            else
            {
                ++it;
            }
        }
    }
}
void ServerApp::debugSelectionButton() {
    std::cout << "button clicked" << std::endl;
}

void ServerApp::onSendToClient(const QString& clientId) {
    std::cout << "[SERVER DEBUG] onSendToClient called for " << clientId.toStdString() << std::endl;
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