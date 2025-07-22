#include "../utils/EnvLoader.h"
#include "../utils/LPTF_Socket.h"
#include "../utils/LPTF_Packet.hpp"
#include "../utils/PacketType.hpp"
#include <vector>
#include <iostream>
#include <windows.h>

using namespace std;

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    cout << "Serveur démarrage..." << endl;

    try {
        LPTF_Socket::initialize();

        LPTF_Socket serveur;
        auto env = EnvLoader::loadEnv("../../.env");
        int port = EnvLoader::loadPort("../../.env");
        serveur.bindSocket(port);
        serveur.listenSocket();

        vector<unique_ptr<LPTF_Socket>> clients;

        cout << "Serveur prêt. En attente de connexions..." << endl;

        while (true) {
            fd_set readfds;
            FD_ZERO(&readfds);

            int maxFd = serveur.getSocketFd();
            FD_SET(serveur.getSocketFd(), &readfds);

            for (const auto& client : clients) {
                int fd = client->getSocketFd();
                FD_SET(fd, &readfds);
                if (fd > maxFd) maxFd = fd;
            }

            // Wait for activity on sockets
            int activity = select(maxFd + 1, &readfds, nullptr, nullptr, nullptr);
            if (activity < 0) {
                cerr << "Erreur lors de select()" << endl;
                break;
            }

            if (FD_ISSET(serveur.getSocketFd(), &readfds)) {
                auto newClient = serveur.acceptSocket();
                cout << "Nouveau client : " << newClient->getClientIP() << endl;
                clients.push_back(move(newClient));
            }

            for (auto it = clients.begin(); it != clients.end(); ) {
                int fd = (*it)->getSocketFd();
                if (FD_ISSET(fd, &readfds)) {
                    try {
                        std::vector<uint8_t> data = (*it)->recvBinary();
                        auto packet = LPTF_Packet::deserialize(data);

                        if (packet.getType() == PacketType::GET_INFO) {
                            std::string payload(packet.getPayload().begin(), packet.getPayload().end());
                            std::cout << "Message client : " << payload << std::endl;

                            std::string response = "Reçu : " + payload;
                            std::vector<uint8_t> responsePayload(response.begin(), response.end());

                            LPTF_Packet responsePacket(1, PacketType::RESPONSE, 0, 1, 1, responsePayload);
                            (*it)->sendBinary(responsePacket.serialize());
                        }
                        ++it;
                    } catch (const std::exception& e) {
                        std::cerr << "Client déconnecté ou erreur : " << e.what() << std::endl;
                        it = clients.erase(it);
                    }
                } else {
                    ++it;
                }
            }
        }

    } catch (const exception& e) {
        cerr << "Erreur fatale : " << e.what() << endl;
        return 1;
    }

    return 0;
}
