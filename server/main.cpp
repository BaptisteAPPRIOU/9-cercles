#include "../utils/LPTF/LPTF_Socket.hpp"
#include "ServerApp.hpp"

#include <QApplication>
#include "MainWindow.hpp"

#include <thread>
#include <iostream>
#include <windows.h>

int main(int argc, char *argv[])
{
    // Démarrage de l'IHM Qt
    QApplication app(argc, argv);
    MainWindow w;
    w.show();

    // Réattache une console pour afficher std::cout / std::cerr
#ifdef _WIN32
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
#endif

    // Initialisation de Winsock avant toute création de socket
    try {
        LPTF_Socket::initialize();
    } catch (const std::exception& e) {
        std::cerr << "Winsock init failed: " << e.what() << std::endl;
        return 1;
    }

    // Lance le serveur en arrière-plan
    std::thread([&w](){
        try {
            ServerApp server("../../.env", &w);
            server.run();
        } catch (const std::exception& e) {
            std::cerr << "Server exception: " << e.what() << std::endl;
        }
    }).detach();

    return app.exec();
}
