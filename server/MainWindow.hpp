#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include "../utils/LPTF/LPTF_Socket.hpp"
#include "ui_MainWindow.h" // generated from MainWindow.ui
#include <QMainWindow>
#include <QListWidget>
#include <QStringList>
#include <QMap>
#include <QMessageBox>

#include <vector>
#include <memory>

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void addClientTab(const QString &clientId);
    void appendClientOutput(const QString &clientId, const QString &text);
    void refreshClients();

    // Call this after constructing MainWindow to connect to ServerApp
    void connectToServerApp(QObject *serverApp);

public slots:
    void onSelectionButtonClicked(bool checked = false);
    void onClientConnected(const QString &clientInfo);

signals:
    void selectionButtonClicked();
    void sendToClient(const QString &clientId);

private:
    Ui::MainWindow *ui;
    QMap<QString, QListWidget *> clientTabs;
};

#endif // MAINWINDOW_HPP
