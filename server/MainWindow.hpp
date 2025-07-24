#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <vector>
#include <memory>
#include "../utils/LPTF_Socket.hpp"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    // explicit MainWindow(std::vector<std::unique_ptr<LPTF_Socket>>& clients, QWidget* parent = nullptr);
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    void refreshClients();

private:
    Ui::MainWindow* ui;
    // std::vector<std::unique_ptr<LPTF_Socket>>& clients;
};

#endif // MAINWINDOW_HPP
