#include "MainWindow.hpp"
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->selectionButton, &QPushButton::clicked,
            this, &MainWindow::onSelectionButtonClicked);

    // Si tu veux prémapper le premier onglet statique (optionnel)
    const QString firstLabel = ui->tabWidget->tabText(0);
    clientTabs[firstLabel] = ui->listWidget;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onClientConnected(const QString& clientInfo, uint32_t sessionId)
{
    m_sessionIds[clientInfo] = sessionId;

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
    addClientTab(clientInfo);
}



void MainWindow::addClientTab(const QString& clientId) {
    if (clientTabs.contains(clientId)) return;

    QWidget* page = new QWidget;
    auto layout = new QVBoxLayout(page);
    layout->setContentsMargins(11, 20, 12, 49);

    QListWidget* outList = new QListWidget;
    layout->addWidget(outList);

    int idx = ui->tabWidget->addTab(page, clientId);
    ui->tabWidget->setCurrentIndex(idx);

    clientTabs[clientId] = outList;
}

void MainWindow::appendClientOutput(const QString& clientId, const QString& text) {
    qDebug() << "[GUI] appendClientOutput for" << clientId << "len=" << text.size();
    if (!clientTabs.contains(clientId)) return;
    QListWidget* list = clientTabs[clientId];
    list->clear();
    const QStringList lines = text.split(u'\n', Qt::SkipEmptyParts);
    for (const QString& line : lines) {
        list->addItem(line);
    }
}

void MainWindow::onClientResponse(const QString& clientId, const QString& text) {
    qDebug() << "[GUI] onClientResponse for" << clientId << "text len=" << text.size();
    appendClientOutput(clientId, text);
}

void MainWindow::onSelectionButtonClicked() {
    int idx = ui->comboBox->currentIndex();
    QListWidgetItem* selectedItem = ui->clientListWidget->currentItem();
    if (!selectedItem) {
        QMessageBox::warning(this, "Attention", "Veuillez sélectionner un client !");
        return;
    }
    QString clientId = selectedItem->text();
    uint32_t sessionId = m_sessionIds.value(clientId, 0);

    qDebug() << "[GUI] onSelectionButtonClicked idx=" << idx << "client=" << clientId;

    switch (idx) {
        case 0: // Afficher les informations du client
            emit getInfoSys(clientId);
            break;
        case 3: // Liste complète des processus
            emit requestProcessList(clientId, false);
            break;
        case 4: // Noms seulement
            emit requestProcessList(clientId, true);
            break;
        default:
            // autres cases non implémentées ici
            break;
    }
}

void MainWindow::refreshClients() {
    // implémentation si besoin
}
