#include "TestScriptRunner.h"
#include "EnvLoader.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>
#include <algorithm>

TestScriptRunner::TestScriptRunner(const std::string& ip, int port, bool verbose_mode, const std::string& id) 
    : server_ip(ip), server_port(port), connected(false), verbose(verbose_mode), client_id(id) {
    LPTF_Socket::initialize();
}

TestScriptRunner::~TestScriptRunner() {
    if (connected && client) {
        // Socket will be closed automatically by LPTF_Socket destructor
        client.reset();
    }
}

TestResult TestScriptRunner::executeCommand(const TestCommand& cmd) {
    if (verbose) {
        std::cout << "Executing: " << cmd.action << " " << cmd.data << std::endl;
    }
    
    if (cmd.action == "connect") {
        return connect();
    }
    else if (cmd.action == "disconnect") {
        return disconnect();
    }
    else if (cmd.action == "send") {
        return sendMessage(cmd.data);
    }
    else if (cmd.action == "expect") {
        return expectMessage(cmd.data, cmd.timeout_ms);
    }
    else if (cmd.action == "wait") {
        return waitMs(std::stoi(cmd.data));
    }
    else {
        return TestResult(false, "Unknown command: " + cmd.action);
    }
}

std::vector<TestResult> TestScriptRunner::runScript(const std::vector<TestCommand>& commands) {
    std::vector<TestResult> results;
    
    for (const auto& cmd : commands) {
        TestResult result = executeCommand(cmd);
        results.push_back(result);
        
        if (!result.success) {
            std::cout << "Test failed: " << result.message << std::endl;
            // Continue with remaining tests or break here if you want to stop on first failure
        }
    }
    
    return results;
}

TestResult TestScriptRunner::connect() {
    try {
        client = std::make_unique<LPTF_Socket>();
        client->connectSocket(server_ip, server_port);
        connected = true;
        return TestResult(true, "Connected successfully");
    }
    catch (const std::exception& e) {
        return TestResult(false, "Connection failed: " + std::string(e.what()));
    }
}

TestResult TestScriptRunner::disconnect() {
    if (client && connected) {
        try {
            // Socket will be closed automatically by LPTF_Socket destructor
            client.reset();
            connected = false;
            return TestResult(true, "Disconnected successfully");
        }
        catch (const std::exception& e) {
            return TestResult(false, "Disconnect failed: " + std::string(e.what()));
        }
    }
    return TestResult(false, "Not connected");
}

TestResult TestScriptRunner::sendMessage(const std::string& message) {
    if (!connected || !client) {
        return TestResult(false, "Not connected to server");
    }
    
    try {
        std::string processed_message = substituteClientId(message);
        client->sendMsg(processed_message);
        return TestResult(true, "Message sent: " + processed_message);
    }
    catch (const std::exception& e) {
        return TestResult(false, "Send failed: " + std::string(e.what()));
    }
}

TestResult TestScriptRunner::expectMessage(const std::string& expected, int timeout_ms) {
    if (!connected || !client) {
        return TestResult(false, "Not connected to server");
    }
    
    try {
        // Simple implementation - for production you might want to implement proper timeout
        std::string received = client->recvMsg();
        std::string processed_expected = substituteClientId(expected);
        
        if (received == processed_expected) {
            return TestResult(true, "Received expected message", received);
        }
        else {
            return TestResult(false, "Expected: '" + processed_expected + "' but got: '" + received + "'", received);
        }
    }
    catch (const std::exception& e) {
        return TestResult(false, "Receive failed: " + std::string(e.what()));
    }
}

TestResult TestScriptRunner::waitMs(int milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    return TestResult(true, "Waited " + std::to_string(milliseconds) + "ms");
}

std::vector<TestCommand> TestScriptRunner::loadScriptFromFile(const std::string& filename) {
    std::vector<TestCommand> commands;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Could not open script file: " << filename << std::endl;
        return commands;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        std::istringstream iss(line);
        std::string action, data;
        int timeout = 5000;
        
        iss >> action;
        
        if (action == "wait") {
            iss >> data; // milliseconds as string
        }
        else if (action == "expect" || action == "send") {
            // Read the rest of the line as data (may contain spaces)
            std::getline(iss, data);
            // Trim leading whitespace
            data.erase(0, data.find_first_not_of(" \t"));
        }
        
        commands.emplace_back(action, data, timeout);
    }
    
    return commands;
}

std::vector<TestCommand> TestScriptRunner::parseCommandLineArgs(int argc, char* argv[]) {
    std::vector<TestCommand> commands;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "--connect") {
            commands.emplace_back("connect");
        }
        else if (arg == "--disconnect") {
            commands.emplace_back("disconnect");
        }
        else if (arg == "--send" && i + 1 < argc) {
            commands.emplace_back("send", argv[++i]);
        }
        else if (arg == "--expect" && i + 1 < argc) {
            commands.emplace_back("expect", argv[++i]);
        }
        else if (arg == "--wait" && i + 1 < argc) {
            commands.emplace_back("wait", argv[++i]);
        }
    }
    
    return commands;
}

void TestScriptRunner::printResults(const std::vector<TestResult>& results) {
    int passed = 0;
    int total = results.size();
    
    std::cout << "\n=== Test Results ===" << std::endl;
    
    for (size_t i = 0; i < results.size(); i++) {
        const auto& result = results[i];
        std::cout << "Test " << (i + 1) << ": " 
                  << (result.success ? "PASS" : "FAIL") 
                  << " - " << result.message << std::endl;
        
        if (!result.received_data.empty()) {
            std::cout << "  Received: " << result.received_data << std::endl;
        }
        
        if (result.success) {
            passed++;
        }
    }
    
    std::cout << "\nSummary: " << passed << "/" << total << " tests passed" << std::endl;
}

bool TestScriptRunner::allTestsPassed(const std::vector<TestResult>& results) {
    return std::all_of(results.begin(), results.end(), 
                      [](const TestResult& r) { return r.success; });
}

std::string TestScriptRunner::substituteClientId(const std::string& text) {
    std::string result = text;
    size_t pos = 0;
    while ((pos = result.find("ID_PLACEHOLDER", pos)) != std::string::npos) {
        result.replace(pos, 14, client_id); // 14 is length of "ID_PLACEHOLDER"
        pos += client_id.length();
    }
    return result;
}
