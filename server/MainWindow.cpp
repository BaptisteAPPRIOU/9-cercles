#include "MainWindow.hpp"
#include <QVBoxLayout>
#include "../utils/SystemInfo/SystemInfo.hpp"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    qDebug() << "[DEBUG] MainWindow constructor: before setupUi";
    ui->setupUi(this);
    qDebug() << "[DEBUG] MainWindow constructor: after setupUi";
    qDebug() << "[DEBUG] MainWindow constructor: before connect";
    bool ok = connect(ui->selectionButton, &QPushButton::clicked, this, &MainWindow::onSelectionButtonClicked);
    qDebug() << "[DEBUG] MainWindow constructor: connect result =" << ok;
    connect(ui->selectionButton, &QPushButton::clicked, [](){
        qDebug() << "[DEBUG] Lambda: selectionButton clicked!";
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onClientConnected(const QString& clientInfo)
{
    // Add to client list widget if not already present
    QListWidget* listWidget = ui->clientListWidget;
    bool exists = false;
    for (int i = 0; i < listWidget->count(); ++i) 
    {
        if (listWidget->item(i)->text() == clientInfo) 
        {
            exists = true;
            break;
        }
    }
    if (!exists)
    {
        listWidget->addItem(clientInfo);
    }
    // Optionally, add a tab for the client
    addClientTab(clientInfo);
}

void MainWindow::connectToServerApp(QObject* serverApp) 
{
    // Connect the signal from ServerApp to this slot
    connect(serverApp, SIGNAL(clientConnected(QString)), this, SLOT(onClientConnected(QString)));
}

// void MainWindow::refreshClients() {
//     ui->clientListWidget->clear();
//     QStringList items;
//     // for (const auto& client : clients) {
//     //     items << QString::fromStdString(client->getClientIP());
//     // }
//     ui->clientListWidget->addItems(items);
// }

void MainWindow::addClientTab(const QString& clientId) {
    if (clientTabs.contains(clientId)) return;

    // 1) Create the page widget and layout
    QWidget* page = new QWidget;
    auto layout = new QVBoxLayout(page);
    layout->setContentsMargins(11, 20, 12, 49);

    // 2) The output area
    QListWidget* outList = new QListWidget;
    layout->addWidget(outList);

    // 4) Add it to the QTabWidget
    int idx = ui->tabWidget->addTab(page, clientId);
    ui->tabWidget->setCurrentIndex(idx);

    // 5) Save the QListWidget for future updates
    clientTabs[clientId] = outList;

    // emit getInfoSys(clientId, msg);  // you define a signal getInfoSys(...)
}

void MainWindow::appendClientOutput(const QString& userAndIp, const QString& text) {
    if (!clientTabs.contains(userAndIp)) return;
    clientTabs[userAndIp]->addItem(text);
}

void MainWindow::onClientResponse(const QString& userAndIp, const QString& response) {
    addResponseToTab(userAndIp, response);
}

void MainWindow::addResponseToTab(const QString& userAndIp, const QString& response) {
    if (!clientTabs.contains(userAndIp)) return;
    clientTabs[userAndIp]->addItem(response);
}

void MainWindow::onSelectionButtonClicked(bool) {
    qDebug() << "[DEBUG] onSelectionButtonClicked called";
    qDebug() << "[DEBUG] About to emit selectionButtonClicked";
    emit selectionButtonClicked();
    int idx = ui->comboBox->currentIndex();
    QListWidgetItem* selectedItem = ui->clientListWidget->currentItem();
    if (!selectedItem) {
        QMessageBox::warning(this, "Attention", "Veuillez sélectionner un client !");
        return;
    }
    QString selectedClientFull = selectedItem->text();
    // Assume format: username + ' ' + ip, extract only the IP
    QString selectedClient;
    int lastSpaceIdx = selectedClientFull.lastIndexOf(' ');
    if (lastSpaceIdx != -1) {
        selectedClient = selectedClientFull.mid(lastSpaceIdx + 1);
    } else {
        selectedClient = selectedClientFull;
    }
    switch (idx) {
        case 0: {
            qDebug() << "[DEBUG] Case 0: Requesting client system info for" << selectedClient;
            emit getInfoSys(selectedClient); // Empty message, just a trigger
            qDebug() << "[DEBUG] Emitted getInfoSys for" << selectedClient;
            break;
        }
        case 1:
            // Démarrer le keylogger
            break;
        case 2:
            // Eteindre le keylogger
            break;
        case 3:
            // Afficher la liste complète des processus
            break;
        case 4:
            // Afficher la liste des processus (noms)
            break;
        case 5:
            // Exécuter une commande
            break;
        default:
            break;
    }
}