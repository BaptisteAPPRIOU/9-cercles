#include "../utils/EnvLoader.hpp"
#include "../utils/LPTF_Socket.hpp"
#include "../utils/LPTF_Packet.hpp"
#include "Menu.hpp"
#include <vector>
#include <iostream>
#include <windows.h>
#include <QApplication>
#include "MainWindow.hpp"

bool isClientConnected = false;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow w;
    w.show();
    return app.exec();

    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    std::cout << "Serveur démarrage…" << std::endl;

    // try
    // {
    //     LPTF_Socket::initialize();

    //     LPTF_Socket serveur;
    //     auto env = EnvLoader::loadEnv("../../.env");
    //     int port = EnvLoader::loadPort("../../.env");
    //     serveur.bindSocket(port);
    //     serveur.listenSocket();

    //     std::vector<std::unique_ptr<LPTF_Socket>> clients;

    //     // Accept one client for UI demonstration
    //     auto newClient = serveur.acceptSocket();
    //     clients.push_back(std::move(newClient));

    //     // Create and show main window
        

    //     std::cout << "Serveur prêt. En attente de connexions…" << std::endl;

    //     while (true)
    //     {
    //         fd_set readfds;
    //         FD_ZERO(&readfds);

    //         int maxFd = serveur.getSocketFd();
    //         FD_SET(serveur.getSocketFd(), &readfds);

    //         for (const auto &client : clients)
    //         {
    //             int fd = client->getSocketFd();
    //             FD_SET(fd, &readfds);
    //             if (fd > maxFd)
    //                 maxFd = fd;
    //         }

    //         int activity = select(maxFd + 1, &readfds, nullptr, nullptr, nullptr);
    //         if (activity < 0)
    //         {
    //             std::cerr << "Erreur lors de select()" << std::endl;
    //             break;
    //         }

    //         // Handle new connections
    //         if (FD_ISSET(serveur.getSocketFd(), &readfds))
    //         {
    //             auto newClient = serveur.acceptSocket();
    //             std::cout << "Nouveau client connecté: " << newClient->getClientIP() << std::endl;
    //             std::cout << "En attente des informations système du client…" << std::endl;

    //             clients.push_back(std::move(newClient));
    //             isClientConnected = true;
    //         }

    //         // Handle client messages
    //         for (auto it = clients.begin(); it != clients.end();)
    //         {
    //             int fd = (*it)->getSocketFd();
    //             if (FD_ISSET(fd, &readfds))
    //             {
    //                 try
    //                 {
    //                     std::vector<uint8_t> data = (*it)->recvBinary();
    //                     auto packet = LPTF_Packet::deserialize(data);

    //                     if (packet.getType() == PacketType::GET_INFO)
    //                     {
    //                         std::string payload(packet.getPayload().begin(), packet.getPayload().end());

    //                         if (payload.find("{") != std::string::npos && payload.find("}") != std::string::npos)
    //                         {
    //                             std::cout << "=== INFORMATIONS SYSTÈME REÇUES ===" << std::endl;
    //                             std::cout << payload << std::endl;
    //                             std::cout << "===================================" << std::endl;

    //                             std::string ack = "Informations système reçues avec succès";
    //                             std::vector<uint8_t> ackPayload(ack.begin(), ack.end());
    //                             LPTF_Packet ackPacket(1, PacketType::RESPONSE, 0, 1, 1, ackPayload);
    //                             (*it)->sendBinary(ackPacket.serialize());
    //                         }
    //                         else
    //                         {
    //                             std::cout << "Message client (" << (*it)->getClientIP() << "): " << payload << std::endl;

    //                             std::string response = "Reçu : " + payload;
    //                             std::vector<uint8_t> responsePayload(response.begin(), response.end());
    //                             LPTF_Packet responsePacket(1, PacketType::RESPONSE, 0, 1, 1, responsePayload);
    //                             (*it)->sendBinary(responsePacket.serialize());
    //                         }
    //                     }
    //                     ++it;
    //                 }
    //                 catch (const std::exception &e)
    //                 {
    //                     std::cout << "Client déconnecté (" << (*it)->getClientIP() << "): " << e.what() << std::endl;
    //                     it = clients.erase(it);
    //                 }
    //             }
    //             else
    //             {
    //                 ++it;
    //             }
    //         }

    //         if (isClientConnected) {
    //             Menu menu(clients);
    //             return menu.run();
    //         }
    //     }
    // }
    // catch (const std::exception &e)
    // {
    //     std::cerr << "Erreur fatale : " << e.what() << std::endl;
    //     return 1;
    // }

    // return 0;
}
