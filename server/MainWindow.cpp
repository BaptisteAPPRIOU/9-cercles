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

void MainWindow::addClientTab(const QString& clientId) {
    if (clientTabs.contains(clientId)) return;

    // 1) Créer le widget de page et son layout
    QWidget* page = new QWidget;
    QVBoxLayout* vlay = new QVBoxLayout(page);

    // 2) La zone d’output
    QListWidget* outList = new QListWidget;
    vlay->addWidget(outList);

    // 3) La ligne de commande + bouton d’envoi
    QHBoxLayout* hlay = new QHBoxLayout;
    QLabel* lbl = new QLabel("Message :");
    QLineEdit* edit = new QLineEdit;
    QPushButton* btn = new QPushButton("Envoyer");
    hlay->addWidget(lbl);
    hlay->addWidget(edit);
    hlay->addWidget(btn);
    vlay->addLayout(hlay);

    // 4) Ajouter dans le QTabWidget
    int idx = ui->tabWidget->addTab(page, clientId);
    ui->tabWidget->setCurrentIndex(idx);

    // 5) Sauvegarder la QListWidget pour mises à jour futures
    clientTabs[clientId] = outList;

    // 6) Connecter le bouton pour émettre un signal vers ton contrôleur réseau
    connect(btn, &QPushButton::clicked, this, [this, clientId, edit]() {
        QString msg = edit->text();
        if (msg.isEmpty()) return;
        emit sendToClient(clientId, msg);  // tu définis un signal sendToClient(...)
        edit->clear();
    });
}

void MainWindow::appendClientOutput(const QString& clientId, const QString& text) {
    if (!clientTabs.contains(clientId)) return;
    clientTabs[clientId]->addItem(text);
}
