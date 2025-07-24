#include "EnvLoader.hpp"

// Load environment variables from a file
std::map<std::string, std::string> EnvLoader::loadEnv(const std::string& path) {
    std::ifstream file(path);
    std::map<std::string, std::string> env;
    std::string line;

    while (getline(file, line)) {
        auto delimiterPos = line.find('=');
        if (delimiterPos == std::string::npos) continue;
        std::string key = line.substr(0, delimiterPos);
        std::string value = line.substr(delimiterPos + 1);
        env[key] = value;
    }

    return env;
}

// Load IP address from environment file
std::string EnvLoader::loadIP(const std::string& filepath) {
    auto env = loadEnv(filepath);
    auto it = env.find("IP");
    if (it == env.end()) {
        throw std::runtime_error("EnvLoader: 'IP' not found in env file");
    }
    return it->second;
}

// Load port from environment file
int EnvLoader::loadPort(const std::string& filepath) {
    auto env = loadEnv(filepath);
    auto it = env.find("PORT");
    if (it == env.end()) {
        throw std::runtime_error("EnvLoader: 'PORT' not found in env file");
    }
    try {
        return std::stoi(it->second);
    } catch (...) {
        throw std::runtime_error(std::string("EnvLoader: invalid PORT value: ") + it->second);
    }
}