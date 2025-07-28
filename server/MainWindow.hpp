#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QListWidget>
#include <QMap>
#include <QMessageBox>

#include <vector>
#include <memory>
#include "../utils/LPTF/LPTF_Socket.hpp"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    void addClientTab(const QString& clientId);
    void appendClientOutput(const QString& clientId, const QString& text);
    void refreshClients();

private:
    Ui::MainWindow* ui;
    QMap<QString, QListWidget*> clientTabs;

signals:
  void sendToClient(const QString& clientId, const QString& message);


private slots:
    void onSelectionButtonClicked();
};

#endif // MAINWINDOW_HPP
