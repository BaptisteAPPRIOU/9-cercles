#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include "../utils/LPTF/LPTF_Socket.hpp"
#include "ui_MainWindow.h" // generated from MainWindow.ui
#include <QMainWindow>
#include <QListWidget>
#include <QStringList>
#include <QMap>
#include <QMessageBox>
#include <QString>
#include <QByteArray>
#include <QPushButton>
#include <QDebug>

#include "../utils/LPTF/LPTF_Packet.hpp"
#include "../utils/LPTF/LPTF_PacketType.hpp"

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
    void refreshClients();

signals:
    void sendToClient(const QString &clientId, const QByteArray &data);
    void getInfoSys(const QString &clientId);
    void requestProcessList(const QString &clientId, bool namesOnly);

public slots:
    void onSelectionButtonClicked();
    void onClientConnected(const QString &clientInfo, uint32_t sessionId);
    void onClientResponse(const QString& clientId, const QString& text);
    void appendClientOutput(const QString &clientId, const QString &text);

private:
    Ui::MainWindow *ui;
    QMap<QString, QListWidget *> clientTabs;
    QMap<QString, uint32_t> m_sessionIds;   // sessionId par clientId
    uint32_t               m_nextPacketId = 0;
};

#endif // MAINWINDOW_HPP
