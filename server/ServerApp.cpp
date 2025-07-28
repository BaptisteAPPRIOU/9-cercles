#include "ServerApp.hpp"
#include "MainWindow.hpp"

#include <QMetaObject>
#include <QString>
#include <QObject>
#include <QDebug>

#include <iostream>
#include <memory>
#include <stdexcept>
#include <cstring>

#ifdef _WIN32
  #include <winsock2.h>   // for select(), FD_* on Windows
  #include <ws2tcpip.h>
  #pragma comment(lib, "ws2_32.lib")
#else
  #include <sys/select.h>
#endif

ServerApp::ServerApp(const std::string& envFilePath, MainWindow* ui)
  : m_envPath(envFilePath)
  , m_ui(ui)
{
    // Console en UTF-8
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    // Charger .env et récupérer le port
    EnvLoader::loadEnv(m_envPath);
    int port = EnvLoader::loadPort(m_envPath);
    std::cout << "ServerApp: binding on port " << port << "...\n";

    // Initialiser Winsock et le socket serveur
#ifdef _WIN32
    LPTF_Socket::initialize();
#else
    LPTF_Socket::initialize();
#endif
    m_serverSocket.bindSocket(port);
    m_serverSocket.listenSocket();
}

ServerApp::~ServerApp() {
    m_serverSocket.closeSocket();
}

int ServerApp::run() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    try {
        qDebug() << "Serveur prêt. En attente de connexions…\n";

        while (true) {
            // Préparation des fd_set
            fd_set readfds;
            FD_ZERO(&readfds);

            int listenFd = m_serverSocket.getSocketFd();
            FD_SET(listenFd, &readfds);
            int maxFd = listenFd;

            for (const auto& client : m_clients) {
                int fd = client->getSocketFd();
                FD_SET(fd, &readfds);
                if (fd > maxFd) maxFd = fd;
            }

            int activity = select(maxFd + 1, &readfds, nullptr, nullptr, nullptr);
            if (activity < 0) {
                std::cerr << "Erreur lors de select(): " << std::strerror(errno) << "\n";
                break;
            }

            // Nouvelle connexion
            if (FD_ISSET(listenFd, &readfds)) {
                qDebug() << "Print new connect";
                auto newClient = m_serverSocket.acceptSocket();
                std::string ip = newClient->getClientIP();
                m_clients.push_back(std::move(newClient));

                if (m_ui) {
                    QString qip = QString::fromStdString(ip);
                    QMetaObject::invokeMethod(
                        m_ui,
                        "addClientTab",
                        Qt::QueuedConnection,
                        Q_ARG(QString, qip)
                    );
                }
            }

            // Lecture et dispatch des messages
            for (auto it = m_clients.begin(); it != m_clients.end();) {
                int fd = (*it)->getSocketFd();
                if (FD_ISSET(fd, &readfds)) {
                    try {
                        auto data = (*it)->recvBinary();
                        auto packet = LPTF_Packet::deserialize(data);
                        std::string payload(packet.getPayload().begin(), packet.getPayload().end());

                        if (m_ui) {
                            QString qip = QString::fromStdString((*it)->getClientIP());
                            QString qtxt = QString::fromStdString(payload);
                            QMetaObject::invokeMethod(
                                m_ui,
                                "appendClientOutput",
                                Qt::QueuedConnection,
                                Q_ARG(QString, qip),
                                Q_ARG(QString, qtxt)
                            );
                        }

                        // Echo/Ack
                        std::string response = "Reçu : " + payload;
                        std::vector<uint8_t> respPayload(response.begin(), response.end());

                        // on construit le paquet avec TOUS les paramètres du constructeur :
                        LPTF_Packet respPkt(
                            1,                      // version
                            PacketType::RESPONSE,   // type
                            0,                      // flags
                            0,                      // packetId
                            0,                      // sessionId
                            respPayload            // payload
                        );
                        (*it)->sendBinary(respPkt.serialize());

                        ++it;
                    } catch (const std::exception& e) {
                        std::string ip = (*it)->getClientIP();
                        std::cout << "Client déconnecté (" << ip << "): " << e.what() << "\n";
                        it = m_clients.erase(it);
                    }
                } else {
                    ++it;
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Erreur fatale dans ServerApp : " << e.what() << "\n";
        return 1;
    }

    return 0;
}
