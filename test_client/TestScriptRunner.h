#ifndef TESTSCRIPTRUNNER_H
#define TESTSCRIPTRUNNER_H

#include "LPTF_Socket.h"
#include <vector>
#include <string>
#include <memory>

struct TestCommand {
    std::string action;  // "send", "expect", "wait", "connect", "disconnect"
    std::string data;    // message data or expected response
    int timeout_ms;      // timeout in milliseconds
    
    TestCommand(const std::string& act, const std::string& d = "", int timeout = 5000) 
        : action(act), data(d), timeout_ms(timeout) {}
};

struct TestResult {
    bool success;
    std::string message;
    std::string received_data;
    
    TestResult(bool s, const std::string& msg, const std::string& data = "") 
        : success(s), message(msg), received_data(data) {}
};

class TestScriptRunner {
private:
    std::unique_ptr<LPTF_Socket> client;
    std::string server_ip;
    int server_port;
    bool connected;
    bool verbose;
    std::string client_id;
    
public:
    TestScriptRunner(const std::string& ip, int port, bool verbose_mode = false, const std::string& id = "");
    ~TestScriptRunner();
    
    // Command execution
    TestResult executeCommand(const TestCommand& cmd);
    std::vector<TestResult> runScript(const std::vector<TestCommand>& commands);
    
    // Script loading
    std::vector<TestCommand> loadScriptFromFile(const std::string& filename);
    std::vector<TestCommand> parseCommandLineArgs(int argc, char* argv[]);
    
    // String processing
    std::string substituteClientId(const std::string& text);
    
    // Utility methods
    TestResult connect();
    TestResult disconnect();
    TestResult sendMessage(const std::string& message);
    TestResult expectMessage(const std::string& expected, int timeout_ms = 5000);
    TestResult waitMs(int milliseconds);
    
    // Output
    void printResults(const std::vector<TestResult>& results);
    bool allTestsPassed(const std::vector<TestResult>& results);
};

#endif // TESTSCRIPTRUNNER_H
