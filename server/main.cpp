#include "../utils/EnvLoader.hpp"
#include "../utils/LPTF_Socket.hpp"
#include "../utils/LPTF_Packet.hpp"
#include "../utils/Hash_Sha256.hpp"
#include "../utils/LPTF_PacketUtils.hpp"


#include "Menu.hpp"
#include <vector>
#include <iostream>
#include <windows.h>
#include <random>

using namespace std;

bool isClientConnected = false;

void send_auth_challenge(LPTF_Socket& socket, uint32_t sessionId, uint16_t packetId, std::string& out_challenge) {
    // Generate random challenge (32 hex chars)
    std::random_device rd; std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    std::stringstream ss;
    for(int i = 0; i < 32; ++i) ss << std::hex << dis(gen);
    out_challenge = ss.str();

    std::vector<uint8_t> payload;
    appendStringToPayload(payload, out_challenge);
    LPTF_Packet pkt(1, PacketType::AUTH_CHALLENGE, 0, packetId, sessionId, payload);
    socket.sendBinary(pkt.serialize());
}

// After receiving AUTH_PROOF:
bool verify_auth_proof(const std::vector<uint8_t>& payload, const std::string& challenge, const std::string& handshake_secret) {
    size_t offset = 0;
    std::string proof = extractStringFromPayload(payload, offset);
    std::string expected = sha256_hex(challenge + handshake_secret);
    return (proof == expected);
}

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

        std::string handshake_secret;
        auto it = env.find("HANDSHAKE_SECRET");
        if (it != env.end()) {
            handshake_secret = it->second;
        } else {
            cerr << "HANDSHAKE_SECRET not found in .env!" << endl;
            return 1;
        }
        serveur.bindSocket(port);
        serveur.listenSocket();

        vector<unique_ptr<LPTF_Socket>> clients;
        std::map<int, std::string> clientChallenges;
        std::map<int, bool> clientAuthenticated;

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
                int clientFd = newClient->getSocketFd();
                cout << "Nouveau client connecté: " << newClient->getClientIP() << endl;
                cout << "En attente des informations système du client..." << endl;

                // Send authentication challenge
                std::string challenge;
                send_auth_challenge(*newClient, 1, 1, challenge);
                clientChallenges[clientFd] = challenge;
                clientAuthenticated[clientFd] = false;

                // No need to send GET_INFO - client will send info automatically
                clients.push_back(move(newClient));
                isClientConnected = true;
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

                        // AUTHENTICATION PHASE
                        if (!clientAuthenticated[fd]) {
                            if (packet.getType() == PacketType::AUTH_PROOF) {
                                if (verify_auth_proof(packet.getPayload(), clientChallenges[fd], handshake_secret)) {
                                    clientAuthenticated[fd] = true;
                                    cout << "Client authentifié: " << (*it)->getClientIP() << endl;
                                    // Optionally send a success response here
                                } else {
                                    cout << "Échec de l'authentification du client: " << (*it)->getClientIP() << endl;
                                    it = clients.erase(it);
                                    continue;
                                }
                            } else {
                                // Ignore all other packets until authenticated
                                ++it;
                                continue;
                            }
                        } else {
                            // AUTHENTICATED: handle other packets
                            if (packet.getType() == PacketType::GET_INFO)
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
                                    LPTF_Packet ackPacket(1, PacketType::RESPONSE, 0, 1, 1, ackPayload);
                                    (*it)->sendBinary(ackPacket.serialize());
                                }
                                else
                                {
                                    cout << "Message client (" << (*it)->getClientIP() << "): " << payload << endl;

                                    // Send regular response
                                    string response = "Reçu : " + payload;
                                    vector<uint8_t> responsePayload(response.begin(), response.end());
                                    LPTF_Packet responsePacket(1, PacketType::RESPONSE, 0, 1, 1, responsePayload);
                                    (*it)->sendBinary(responsePacket.serialize());
                                }
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

            if (isClientConnected) {
                Menu menu(clients);
                return menu.run();
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