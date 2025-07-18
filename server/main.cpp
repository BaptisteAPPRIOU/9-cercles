#include "LPTF_Socket.h"
#include <vector>
#include <iostream>
#include "../utils/EnvLoader.h"

int main() {
    std::cout << "Serveur démarrage..." << std::endl;

    try {
        LPTF_Socket::initialize();

        LPTF_Socket serveur;
        auto env = EnvLoader::loadEnv("../.env");
        int port = std::stoi(env["PORT"]);
        serveur.bindSocket(port);
        serveur.listenSocket();

        std::vector<std::unique_ptr<LPTF_Socket>> clients;

        std::cout << "Serveur prêt. En attente de connexions..." << std::endl;

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
                std::cerr << "Erreur lors de select()" << std::endl;
                break;
            }
            if (FD_ISSET(serveur.getSocketFd(), &readfds)) {
                auto newClient = serveur.acceptSocket();
                std::cout << "Nouveau client : " << newClient->getClientIP() << std::endl;
                clients.push_back(std::move(newClient));
            }

            // Messages from existing clients
            for (auto it = clients.begin(); it != clients.end(); ) {
                int fd = (*it)->getSocketFd();
                if (FD_ISSET(fd, &readfds)) {
                    std::string msg = (*it)->recvMsg();
                    if (msg.empty()) {
                        std::cout << "Client déconnecté." << std::endl;
                        it = clients.erase(it);  // removes the client
                        continue;
                    } else {
                        std::cout << "Reçu : " << msg << std::endl;
                        (*it)->sendMsg("Message reçu !");
                    }
                }
                ++it;
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "Erreur fatale : " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
