#ifndef SYSTEMINFO_H
#define SYSTEMINFO_H

#include <string>
#include <map>

class SystemInfo {
public:
    static std::map<std::string, std::string> getSystemInfo();
    static std::string getHostname();
    static std::string getUsername();
    static std::string getOperatingSystem();
    static std::string toJson(const std::map<std::string, std::string>& info);
};
#endif // SYSTEMINFO_H