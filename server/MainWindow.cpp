#include "MainWindow.hpp"
#include "ui_MainWindow.h"  // generated from MainWindow.ui

#include <QStringList>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // Pas de clients pour l'instant → listWidget restera vide
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::refreshClients() {
    ui->listWidget->clear();
    QStringList items;
    // for (const auto& client : clients) {
    //     items << QString::fromStdString(client->getClientIP());
    // }
    ui->listWidget->addItems(items);
}
