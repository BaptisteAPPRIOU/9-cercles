#include "../utils/EnvLoader.hpp"
#include "../utils/LPTF/LPTF_Socket.hpp"
#include "../utils/LPTF/LPTF_Packet.hpp"
#include "ServerApp.hpp"
#include "MainWindow.hpp"

#include <QApplication>
#include <QThread>
#include <thread>
#include <iostream>
#include <windows.h>

int main(int argc, char *argv[])
{
    // 0) Initialize WSA once, before any LPTF_Socket calls
    LPTF_Socket::initialize();

    QApplication app(argc, argv);
    MainWindow w;

    // Create ServerApp and move it to a QThread
    ServerApp *serverApp = new ServerApp("../../.env");
    QThread *serverThread = new QThread;
    serverApp->moveToThread(serverThread);

    // Connect ServerApp signals to UI
    QObject::connect(serverApp, &ServerApp::clientConnected, &w, &MainWindow::onClientConnected);
    QObject::connect(serverApp, &ServerApp::clientResponse, &w, &MainWindow::onClientResponse);
    QObject::connect(&app, &QApplication::aboutToQuit, serverApp, &QObject::deleteLater);
    QObject::connect(&app, &QApplication::aboutToQuit, serverThread, &QThread::quit);
    QObject::connect(serverThread, &QThread::finished, serverThread, &QObject::deleteLater);

    // Connect MainWindow's getInfoSys signal to ServerApp's onGetInfoSys slot
    QObject::connect(&w, &MainWindow::getInfoSys, serverApp, &ServerApp::onGetInfoSys);

    // Start the blocking server loop in the QThread
    QObject::connect(serverThread, &QThread::started, serverApp, [serverApp]()
                     { serverApp->run(); });
    serverThread->start();

    w.show();
    return app.exec();
}
