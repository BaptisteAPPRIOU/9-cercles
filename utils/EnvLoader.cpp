#include <iostream>
#include <fstream>
#include <string>
#include <map>

std::map<std::string, std::string> loadEnv(const std::string& path) {
    std::ifstream file(path);
    std::map<std::string, std::string> env;
    std::string line;

    while (std::getline(file, line)) {
        auto delimiterPos = line.find('=');
        if (delimiterPos == std::string::npos) continue;
        std::string key = line.substr(0, delimiterPos);
        std::string value = line.substr(delimiterPos + 1);
        env[key] = value;
    }

    return env;
}
