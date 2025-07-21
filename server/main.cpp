#include "../utils/EnvLoader.h"
#include "../utils/LPTF_Socket.h"
#include "../utils/LPTF_Packet.h"
#include <vector>
#include <iostream>
#include <windows.h>

using namespace std;

int main()
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    cout << "Serveur démarrage..." << endl;

    try
    {
        LPTF_Socket::initialize();

        LPTF_Socket serveur;
        auto env = EnvLoader::loadEnv("../../.env");
        int port = EnvLoader::loadPort("../../.env");
        serveur.bindSocket(port);
        serveur.listenSocket();

        vector<unique_ptr<LPTF_Socket>> clients;

        cout << "Serveur prêt. En attente de connexions..." << endl;

        while (true)
        {
            fd_set readfds;
            FD_ZERO(&readfds);

            int maxFd = serveur.getSocketFd();
            FD_SET(serveur.getSocketFd(), &readfds);

            for (const auto &client : clients)
            {
                int fd = client->getSocketFd();
                FD_SET(fd, &readfds);
                if (fd > maxFd)
                    maxFd = fd;
            }

            int activity = select(maxFd + 1, &readfds, nullptr, nullptr, nullptr);
            if (activity < 0)
            {
                cerr << "Erreur lors de select()" << endl;
                break;
            }

            // Handle new connections
            if (FD_ISSET(serveur.getSocketFd(), &readfds))
            {
                auto newClient = serveur.acceptSocket();
                cout << "Nouveau client connecté: " << newClient->getClientIP() << endl;
                cout << "En attente des informations système du client..." << endl;

                // No need to send GET_INFO - client will send info automatically
                clients.push_back(move(newClient));
            }

            // Handle client messages
            for (auto it = clients.begin(); it != clients.end();)
            {
                int fd = (*it)->getSocketFd();
                if (FD_ISSET(fd, &readfds))
                {
                    try
                    {
                        vector<uint8_t> data = (*it)->recvBinary();
                        auto packet = LPTF_Packet::deserialize(data);

                        if (packet.getType() == LPTF_Packet::GET_INFO)
                        {
                            string payload(packet.getPayload().begin(), packet.getPayload().end());

                            // Check if it's JSON (system info) or regular message
                            if (payload.find("{") != string::npos && payload.find("}") != string::npos)
                            {
                                cout << "=== INFORMATIONS SYSTÈME REÇUES ===" << endl;
                                cout << payload << endl;
                                cout << "===================================" << endl;

                                // Send acknowledgment
                                string ack = "Informations système reçues avec succès";
                                vector<uint8_t> ackPayload(ack.begin(), ack.end());
                                LPTF_Packet ackPacket(1, LPTF_Packet::RESPONSE, ackPayload);
                                (*it)->sendBinary(ackPacket.serialize());
                            }
                            else
                            {
                                cout << "Message client (" << (*it)->getClientIP() << "): " << payload << endl;

                                // Send regular response
                                string response = "Reçu : " + payload;
                                vector<uint8_t> responsePayload(response.begin(), response.end());
                                LPTF_Packet responsePacket(1, LPTF_Packet::RESPONSE, responsePayload);
                                (*it)->sendBinary(responsePacket.serialize());
                            }
                        }
                        ++it;
                    }
                    catch (const exception &e)
                    {
                        cout << "Client déconnecté (" << (*it)->getClientIP() << "): " << e.what() << endl;
                        it = clients.erase(it);
                    }
                }
                else
                {
                    ++it;
                }
            }
        }
    }
    catch (const exception &e)
    {
        cerr << "Erreur fatale : " << e.what() << endl;
        return 1;
    }

    return 0;
}
