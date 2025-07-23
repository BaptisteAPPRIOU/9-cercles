#include "LPTF_Socket.h"
#include <vector>
#include <iostream>
#include "../utils/EnvLoader.h"
using namespace std;

int main(int argc, char* argv[]) {
    cout << "Serveur démarrage..." << endl;

    try {
        LPTF_Socket::initialize();

        std::string envPath = "../../.env";
        if (argc > 1) {
            envPath = argv[1];
            std::cout << "Using custom .env path: " << envPath << std::endl;
        } else {
            std::cout << "Using default .env path: " << envPath << std::endl;
        }
        
        LPTF_Socket serveur;
        auto env = EnvLoader::loadEnv(envPath);
        int port = EnvLoader::loadPort(envPath);
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

            // Messages from existing clients
            for (auto it = clients.begin(); it != clients.end(); ) {
                int fd = (*it)->getSocketFd();
                if (FD_ISSET(fd, &readfds)) {
                    string msg = (*it)->recvMsg();
                    if (msg.empty()) {
                        cout << "Client déconnecté." << endl;
                        it = clients.erase(it);  // removes the client
                        continue;
                    } else {
                        cout << "Reçu : " << msg << endl;
                        (*it)->sendMsg("Message reçu !");
                    }
                }
                ++it;
            }
        }

    } catch (const exception& e) {
        cerr << "Erreur fatale : " << e.what() << endl;
        return 1;
    }

    return 0;
}
