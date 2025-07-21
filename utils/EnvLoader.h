#ifndef ENVLOADER_H
#define ENVLOADER_H

#include <fstream>
#include <string>
#include <map>
#include <stdexcept>
using namespace std;

class EnvLoader {
public:
    static map<string, string> loadEnv(const string& filepath);
    static string loadIP(const string& filepath);
    static int loadPort(const string& filepath);
};

#endif // ENVLOADER_H