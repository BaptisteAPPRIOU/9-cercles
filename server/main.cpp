#include "../utils/EnvLoader.hpp"
#include "../utils/LPTF/LPTF_Socket.hpp"
#include "../utils/LPTF/LPTF_Packet.hpp"
#include "ServerApp.hpp"
#include "MainWindow.hpp"

#include <QApplication>
#include <thread>
#include <iostream>
#include <windows.h>

int main(int argc, char *argv[])
{
    // 0) Initialize WSA once, before any LPTF_Socket calls
    LPTF_Socket::initialize();

    // 1) Launch the server in a detached thread
    std::thread([](){
        ServerApp server("../../.env");  // m_serverSocket will be constructed after WSAStartup
        server.run();
    }).detach();

    // Start the Qt GUI (just the window, without server/UI logic for now)
    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    return app.exec();
}
