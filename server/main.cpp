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

    // Create ServerApp with QThread
    QThread* serverThread = new QThread;
    ServerApp* serverApp = new ServerApp("../../.env");
    serverApp->moveToThread(serverThread);

    // Connect signal/slot with Qt5 syntax
    QObject::connect(serverApp, &ServerApp::clientConnected, &w, &MainWindow::onClientConnected);
    QObject::connect(serverThread, &QThread::started, serverApp, &ServerApp::run);
    QObject::connect(&app, &QApplication::aboutToQuit, serverThread, &QThread::quit);
    QObject::connect(serverThread, &QThread::finished, serverApp, &QObject::deleteLater);
    QObject::connect(serverThread, &QThread::finished, serverThread, &QObject::deleteLater);

    // Connect MainWindow's sendToClient signal to ServerApp's onSendToClient slot
    // bool ok = QObject::connect(&w, &MainWindow::sendToClient, serverApp, &ServerApp::onSendToClient, Qt::QueuedConnection);
    QObject::connect(&w, &MainWindow::sendToClient, serverApp, &ServerApp::onSendToClient);
    // std::cout << "[DEBUG] sendToClient connection result: " << ok << std::endl;

    serverThread->start();

    w.show();
    return app.exec();
}
