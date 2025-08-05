#include "SystemInfo.hpp"

std::map<std::string, std::string> SystemInfo::getSystemInfo()
{
    std::map<std::string, std::string> info;

    info["hostname"] = getHostname();
    info["username"] = getUsername();
    info["operating_system"] = getOperatingSystem();

    return info;
}

std::string SystemInfo::getHostname()
{
#ifdef _WIN32
    char hostname[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(hostname);
    if (GetComputerNameA(hostname, &size))
    {
        return std::string(hostname);
    }
    return "Unknown";
#else
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0)
    {
        return std::string(hostname);
    }
    return "Unknown";
#endif
}

std::string SystemInfo::getUsername()
{
#ifdef _WIN32
    char username[UNLEN + 1];
    DWORD size = sizeof(username);
    if (GetUserNameA(username, &size))
    {
        return std::string(username);
    }
    return "Unknown";
#else
    struct passwd *pw = getpwuid(getuid());
    if (pw)
    {
        return std::string(pw->pw_name);
    }
    return "Unknown";
#endif
}

std::string SystemInfo::getOperatingSystem()
{
#ifdef _WIN32
    // Try registry first
    std::string prodAndBuild = getWindowsProductAndBuild();
    if (!prodAndBuild.empty())
        return prodAndBuild;
    // Fallback
    std::string rtlVersion = getWindowsVersionViaRTL();
    if (!rtlVersion.empty())
        return rtlVersion;
    return "Windows (version unknown)";
#else
    struct utsname unameData;
    if (uname(&unameData) == 0)
    {
        std::ostringstream oss;
        oss << unameData.sysname << " " << unameData.release;
        return oss.str();
    }
    return "Unix-like";
#endif
}

#ifdef _WIN32
std::string SystemInfo::getWindowsProductAndBuild()
{
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                      "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
                      0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        char productName[256] = {0};
        char buildNumber[64] = {0};
        DWORD size = sizeof(productName);
        DWORD buildSize = sizeof(buildNumber);
        bool hasProductName = false;
        bool hasBuildNumber = false;
        std::string result;

        if (RegQueryValueExA(hKey, "ProductName", NULL, NULL, (LPBYTE)productName, &size) == ERROR_SUCCESS)
        {
            result = std::string(productName);
            hasProductName = true;
        }
        if (RegQueryValueExA(hKey, "CurrentBuild", NULL, NULL, (LPBYTE)buildNumber, &buildSize) == ERROR_SUCCESS)
        {
            hasBuildNumber = true;
        }
        RegCloseKey(hKey);

        if (hasProductName)
        {
            if (hasBuildNumber)
            {
                result = smartWindowsVersionName(result, buildNumber);
            }
            return result;
        }
    }
    return "";
}

std::string SystemInfo::smartWindowsVersionName(const std::string &prod, const std::string &buildStr)
{
    int build = std::stoi(buildStr);
    std::string result = prod;
    // Windows 11 detection logic
    if (build >= 22000)
    {
        if (result.find("Windows 10") != std::string::npos)
        {
            if (result.find("Pro") != std::string::npos)
                result = build >= 26000 ? "Windows 11 Pro 24H2" : "Windows 11 Pro";
            else if (result.find("Home") != std::string::npos)
                result = build >= 26000 ? "Windows 11 Home 24H2" : "Windows 11 Home";
            else
                result = build >= 26000 ? "Windows 11 24H2" : "Windows 11";
        }
        result += " (Build " + buildStr + ")";
    }
    else
    {
        result += " (Build " + buildStr + ")";
    }
    return result;
}

std::string SystemInfo::getWindowsVersionViaRTL()
{
    HMODULE hMod = GetModuleHandleW(L"kernel32.dll");
    if (hMod)
    {
        typedef NTSTATUS(WINAPI * RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);
        RtlGetVersionPtr RtlGetVersion = (RtlGetVersionPtr)GetProcAddress(hMod, "RtlGetVersion");
        if (RtlGetVersion)
        {
            RTL_OSVERSIONINFOW osvi = {0};
            osvi.dwOSVersionInfoSize = sizeof(osvi);
            if (RtlGetVersion(&osvi) == 0)
            {
                std::ostringstream oss;
                if (osvi.dwMajorVersion == 10 && osvi.dwBuildNumber >= 22000)
                {
                    if (osvi.dwBuildNumber >= 26000)
                        oss << "Windows 11 24H2 (Build " << osvi.dwBuildNumber << ")";
                    else if (osvi.dwBuildNumber >= 22621)
                        oss << "Windows 11 22H2 (Build " << osvi.dwBuildNumber << ")";
                    else
                        oss << "Windows 11 (Build " << osvi.dwBuildNumber << ")";
                }
                else if (osvi.dwMajorVersion == 10)
                {
                    oss << "Windows 10 (Build " << osvi.dwBuildNumber << ")";
                }
                else
                {
                    oss << "Windows " << osvi.dwMajorVersion << "." << osvi.dwMinorVersion
                        << " (Build " << osvi.dwBuildNumber << ")";
                }
                return oss.str();
            }
        }
    }
    return "";
}
#endif

std::string SystemInfo::toJson(const std::map<std::string, std::string> &info)
{
    std::ostringstream json;
    json << "{";

    bool first = true;
    for (const auto &pair : info)
    {
        if (!first)
            json << ",";
        json << "\"" << pair.first << "\":\"" << pair.second << "\"";
        first = false;
    }

    json << "}";
    return json.str();
}