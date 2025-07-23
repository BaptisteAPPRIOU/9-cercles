#include "EnvLoader.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>

map<string, string> EnvLoader::loadEnv(const string& path) {
    ifstream file(path);
    map<string, string> env;
    string line;

    while (getline(file, line)) {
        auto delimiterPos = line.find('=');
        if (delimiterPos == string::npos) continue;
        string key = line.substr(0, delimiterPos);
        string value = line.substr(delimiterPos + 1);
        env[key] = value;
    }

    return env;
}

string EnvLoader::loadIP(const string& filepath) {
    auto env = loadEnv(filepath);
    auto it = env.find("IP");
    if (it == env.end()) {
        throw runtime_error("EnvLoader: 'IP' not found in env file");
    }
    return it->second;
}

int EnvLoader::loadPort(const string& filepath) {
    auto env = loadEnv(filepath);
    // Debug print: show all loaded keys and values
    std::cout << "EnvLoader debug: loaded keys from " << filepath << std::endl;
    for (const auto& kv : env) {
        std::cout << "  key: [" << kv.first << "] value: [" << kv.second << "]" << std::endl;
    }
    auto it = env.find("PORT");
    if (it == env.end()) {
        throw runtime_error("EnvLoader: 'PORT' not found in env file");
    }
    try {
        return stoi(it->second);
    } catch (...) {
        throw runtime_error(string("EnvLoader: invalid PORT value: ") + it->second);
    }
}