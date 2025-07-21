#ifndef ENVLOADER_H
#define ENVLOADER_H

#include <fstream>
#include <string>
#include <map>

class EnvLoader {
public:
    static std::map<std::string, std::string> loadEnv(const std::string& filepath);
    static std::string loadIP(const std::string& filepath);
    static int loadPort(const std::string& filepath);
};

#endif // ENVLOADER_H