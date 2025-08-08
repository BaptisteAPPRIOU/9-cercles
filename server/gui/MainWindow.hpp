#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include "ui_MainWindow.h"
#include "AnalysisWidget.hpp"
#include <QMainWindow>
#include <QListWidget>
#include <QMap>
#include <QMessageBox>
#include <QString>
#include <QByteArray>
#include <QPushButton>
#include <QDebug>
#include <QVBoxLayout>
#include <QInputDialog>
#include <QMap>
#include <QVariant>

#include "../utils/LPTF/LPTF_Packet.hpp"
#include "../utils/LPTF/LPTF_PacketType.hpp"

// Forward declaration
class LPTF_database;

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
    void executeCommand(const QString &clientId);
    void updateClientStatus(const QString &clientInfo, bool isOnline);
    QListWidgetItem* findClientItem(const QString &clientInfo);
    
    // Set database connection for analysis widget
    void setDatabase(LPTF_database* database);

signals:
    void sendToClient(const QString &clientId, const QByteArray &data);
    void getInfoSys(const QString &clientId);
    void startKeylogger(const QString &clientId);
    void stopKeylogger(const QString &clientId);
    void requestProcessList(const QString &clientId, bool namesOnly);

public slots:
    void onSelectionButtonClicked();
    void onAnalyzeButtonClicked();
    void onClientConnected(const QString &clientInfo, uint32_t sessionId);
    void onClientResponse(const QString &clientId, const QString &text);
    void appendClientOutput(const QString &clientId, const QString &text);
    void onClientDisconnected(const QString &clientInfo);
    void onClientsLoadedFromDatabase(const QList<QMap<QString, QVariant>> &clients);

private:
    Ui::MainWindow *ui;
    QMap<QString, QListWidget *> clientTabs;
    QMap<QString, uint32_t> m_sessionIds;
    uint32_t m_nextPacketId = 0;
    
    // Analysis widget
    AnalysisWidget* analysisWidget_;
    LPTF_database* database_;
};

#endif // MAINWINDOW_HPP
