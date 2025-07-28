#include "../utils/LPTF/LPTF_Socket.hpp"
#include "ServerApp.hpp"
#include <thread>
#include <iostream>
#include <windows.h>

#include <QApplication>
#include "MainWindow.hpp"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 1) Crée l’UI
    MainWindow w;
    w.show();

    // 2) Lance le serveur en arrière‑plan en lui passant &w
    std::thread([&w](){
        std::cout << "Serveur prêt. avant try \n";
        try {
            std::cout << "Serveur prêt. \n";
            ServerApp server("../../.env", &w);
            std::cout << "Serveur prêt2. \n";
            server.run();
            std::cout << "Serveur prêt.3 \n";
        } catch (const std::exception& e) {
            // log si besoin
        }
    }).detach();

    // 3) Boucle Qt
    return app.exec();
}
