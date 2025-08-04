#ifndef SYSTEMINFO_HPP
#define SYSTEMINFO_HPP

#include <map>
#include <string>
#include <sstream>

#ifdef _WIN32
    #include <windows.h>
    #include <lmcons.h>
    #pragma comment(lib, "advapi32.lib")
#else
    #include <unistd.h>
    #include <sys/utsname.h>
    #include <pwd.h>
#endif

class SystemInfo {
public:
    static std::map<std::string, std::string> getSystemInfo();
    static std::string getHostname();
    static std::string getUsername();
    static std::string getOperatingSystem();
    static std::string toJson(const std::map<std::string, std::string>& info);

private:
#ifdef _WIN32
    static std::string getWindowsProductAndBuild();
    static std::string getWindowsVersionViaRTL();
    static std::string smartWindowsVersionName(const std::string& prod, const std::string& buildStr);
#endif
};


#endif // SYSTEMINFO_HPP
