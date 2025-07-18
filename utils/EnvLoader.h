#ifndef ENVLOADER_H
#define ENVLOADER_H

#include <fstream>
#include <string>
#include <map>

class EnvLoader {
public:
    // Charge un fichier .env et retourne un map contenant les variables
    static std::map<std::string, std::string> loadEnv(const std::string& filepath);
};

#endif // ENVLOADER_H