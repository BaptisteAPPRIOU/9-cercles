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


/**
 * @class SystemInfo
 * @brief Provides static methods to retrieve system information and convert it to JSON.
 */
class SystemInfo {
public:
    /**
     * @brief Retrieves a map of system information fields and their values.
     * @return Map with keys such as "hostname", "username", "operating_system".
     */
    static std::map<std::string, std::string> getSystemInfo();

    /**
     * @brief Gets the system's hostname.
     * @return Hostname as a string.
     */
    static std::string getHostname();

    /**
     * @brief Gets the current user's username.
     * @return Username as a string.
     */
    static std::string getUsername();

    /**
     * @brief Gets the operating system name and version.
     * @return Operating system as a string.
     */
    static std::string getOperatingSystem();

    /**
     * @brief Converts a map of system information to a JSON string.
     * @param info Map of system information fields and values.
     * @return JSON representation as a string.
     */
    static std::string toJson(const std::map<std::string, std::string>& info);

private:
#ifdef _WIN32
    /**
     * @brief Gets the Windows product name and build number.
     * @return Product and build as a string.
     */
    static std::string getWindowsProductAndBuild();

    /**
     * @brief Gets the Windows version using RTL functions.
     * @return Windows version as a string.
     */
    static std::string getWindowsVersionViaRTL();

    /**
     * @brief Generates a smart Windows version name from product and build strings.
     * @param prod Product name string.
     * @param buildStr Build number string.
     * @return Smart version name as a string.
     */
    static std::string smartWindowsVersionName(const std::string& prod, const std::string& buildStr);
#endif
};


#endif // SYSTEMINFO_HPP
