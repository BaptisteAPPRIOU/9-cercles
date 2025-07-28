#include "../utils/LPTF/LPTF_Socket.hpp"   // pour LPTF_Socket::initialize()
#include "ServerApp.hpp"
#include <thread>
#include <iostream>
#include <windows.h>

#include <QApplication>
#include "MainWindow.hpp"

int main(int argc, char *argv[])
{
    // 1) Initialisation de Winsock (avant toute socket)
    try {
        LPTF_Socket::initialize();
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal: Winsock init failed: " << e.what() << std::endl;
        return 1;
    }

    // 2) Lance le serveur en arrière‑plan
    std::thread([](){
        try {
            ServerApp server("../../.env");
            server.run();
        } catch (const std::exception& e) {
            // On peut logger dans un fichier, ou simplement ignorer
        }
    }).detach();

    // 3) Démarre l'IHM Qt
    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    return app.exec();
}
