#ifndef ENVLOADER_HPP
#define ENVLOADER_HPP

#include <map>
#include <string>
#include <fstream>
#include <stdexcept>

class EnvLoader {
public:
    static std::map<std::string, std::string> loadEnv(const std::string& filepath);
    static std::string loadIP(const std::string& filepath);
    static int loadPort(const std::string& filepath);
};

#endif // ENVLOADER_HPP
