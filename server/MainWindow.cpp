#include "MainWindow.hpp"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // No clients for now → listWidget will remain empty
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

    // 1) Create the page widget and its layout
    QWidget* page = new QWidget;

    // 2) The output area
    QListWidget* outList = new QListWidget;

    // 4) Add it to the QTabWidget
    int idx = ui->tabWidget->addTab(page, clientId);
    ui->tabWidget->setCurrentIndex(idx);

    // 5) Save the QListWidget for future updates
    clientTabs[clientId] = outList;

    // emit sendToClient(clientId, msg);  // you define a signal sendToClient(...)
}

void MainWindow::appendClientOutput(const QString& clientId, const QString& text) {
    if (!clientTabs.contains(clientId)) return;
    clientTabs[clientId]->addItem(text);
}
