#include "TestScriptRunner.h"
#include "EnvLoader.h"
#include <iostream>
#include <string>

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [OPTIONS] [COMMANDS]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --script <file>     Run commands from script file\n";
    std::cout << "  --ip <address>      Server IP address (default: from .env)\n";
    std::cout << "  --port <number>     Server port (default: from .env)\n";
    std::cout << "  --client-id <id>    Client identifier for message substitution\n";
    std::cout << "  --verbose           Enable verbose output\n";
    std::cout << "  --help              Show this help\n\n";
    std::cout << "Commands:\n";
    std::cout << "  --connect           Connect to server\n";
    std::cout << "  --send <message>    Send message to server\n";
    std::cout << "  --expect <message>  Expect specific response from server\n";
    std::cout << "  --wait <ms>         Wait for specified milliseconds\n";
    std::cout << "  --disconnect        Disconnect from server\n\n";
    std::cout << "Examples:\n";
    std::cout << "  " << programName << " --script test_script.txt\n";
    std::cout << "  " << programName << " --client-id Client1 --script unique_client_test.txt\n";
    std::cout << "  " << programName << " --connect --send \"hello\" --expect \"world\" --disconnect\n";
}

int main(int argc, char* argv[]) {
    try {
        // Default values
        std::string script_file;
        std::string ip;
        int port = 0;
        bool verbose = false;
        bool show_help = false;
        std::string client_id;
        
        // Parse arguments
        std::vector<std::string> args(argv, argv + argc);
        
        for (size_t i = 1; i < args.size(); i++) {
            if (args[i] == "--help") {
                show_help = true;
            }
            else if (args[i] == "--script" && i + 1 < args.size()) {
                script_file = args[++i];
            }
            else if (args[i] == "--ip" && i + 1 < args.size()) {
                ip = args[++i];
            }
            else if (args[i] == "--port" && i + 1 < args.size()) {
                port = std::stoi(args[++i]);
            }
            else if (args[i] == "--client-id" && i + 1 < args.size()) {
                client_id = args[++i];
            }
            else if (args[i] == "--verbose") {
                verbose = true;
            }
        }
        
        if (show_help) {
            printUsage(argv[0]);
            return 0;
        }
        
        // Load default IP and port from .env if not specified
        if (ip.empty() || port == 0) {
            try {
                auto env = EnvLoader::loadEnv("../.env");
                if (ip.empty()) {
                    ip = EnvLoader::loadIP("../.env");
                }
                if (port == 0) {
                    port = EnvLoader::loadPort("../.env");
                }
            }
            catch (const std::exception& e) {
                std::cerr << "Warning: Could not load .env file: " << e.what() << std::endl;
                std::cerr << "Using defaults: localhost:8080" << std::endl;
                if (ip.empty()) ip = "localhost";
                if (port == 0) port = 8080;
            }
        }
        
        std::cout << "Test Client Configuration:" << std::endl;
        std::cout << "Server: " << ip << ":" << port << std::endl;
        std::cout << "Client ID: " << (client_id.empty() ? "none" : client_id) << std::endl;
        std::cout << "Verbose: " << (verbose ? "enabled" : "disabled") << std::endl;
        
        TestScriptRunner runner(ip, port, verbose, client_id);
        std::vector<TestCommand> commands;
        
        if (!script_file.empty()) {
            // Load commands from script file
            std::cout << "Loading script: " << script_file << std::endl;
            commands = runner.loadScriptFromFile(script_file);
            
            if (commands.empty()) {
                std::cerr << "No commands found in script file or file could not be read." << std::endl;
                return 1;
            }
        }
        else {
            // Parse commands from command line
            commands = runner.parseCommandLineArgs(argc, argv);
            
            if (commands.empty()) {
                std::cout << "No commands specified. Use --help for usage information." << std::endl;
                return 0;
            }
        }
        
        // Execute the test script
        std::cout << "\nExecuting " << commands.size() << " commands..." << std::endl;
        auto results = runner.runScript(commands);
        
        // Print results
        runner.printResults(results);
        
        // Return appropriate exit code
        return runner.allTestsPassed(results) ? 0 : 1;
        
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
