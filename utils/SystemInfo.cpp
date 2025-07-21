#include "SystemInfo.h"
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

std::map<std::string, std::string> SystemInfo::getSystemInfo() {
    std::map<std::string, std::string> info;
    
    info["hostname"] = getHostname();
    info["username"] = getUsername();
    info["operating_system"] = getOperatingSystem();
    
    return info;
}

std::string SystemInfo::getHostname() {
#ifdef _WIN32
    char hostname[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(hostname);
    if (GetComputerNameA(hostname, &size)) {
        return std::string(hostname);
    }
    return "Unknown";
#else
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        return std::string(hostname);
    }
    return "Unknown";
#endif
}

std::string SystemInfo::getUsername() {
#ifdef _WIN32
    char username[UNLEN + 1];
    DWORD size = sizeof(username);
    if (GetUserNameA(username, &size)) {
        return std::string(username);
    }
    return "Unknown";
#else
    struct passwd* pw = getpwuid(getuid());
    if (pw) {
        return std::string(pw->pw_name);
    }
    return "Unknown";
#endif
}

std::string SystemInfo::getOperatingSystem() {
    //TODO refactore this function it's too long and complex
#ifdef _WIN32
    // Method 1: Try registry first (most reliable)
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, 
                      "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 
                      0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        
        char productName[256] = {0};
        char buildNumber[64] = {0};
        DWORD size = sizeof(productName);
        DWORD buildSize = sizeof(buildNumber);
        
        std::string result;
        bool hasProductName = false;
        bool hasBuildNumber = false;
        
        // Get product name
        if (RegQueryValueExA(hKey, "ProductName", NULL, NULL, 
                            (LPBYTE)productName, &size) == ERROR_SUCCESS) {
            result = std::string(productName);
            hasProductName = true;
        }
        
        // Get build number
        if (RegQueryValueExA(hKey, "CurrentBuild", NULL, NULL, 
                            (LPBYTE)buildNumber, &buildSize) == ERROR_SUCCESS) {
            hasBuildNumber = true;
        }
        
        RegCloseKey(hKey);
        
        // If we have build number, check if it's Windows 11
        if (hasBuildNumber) {
            int build = std::stoi(std::string(buildNumber));
            
            // Windows 11 detection based on build number
            if (build >= 22000) {
                // Override product name if it incorrectly says Windows 10
                if (result.find("Windows 10") != std::string::npos) {
                    // Try to preserve the edition (Pro, Home, etc.)
                    if (result.find("Pro") != std::string::npos) {
                        if (build >= 26000) {
                            result = "Windows 11 Pro 24H2";
                        } else {
                            result = "Windows 11 Pro";
                        }
                    } else if (result.find("Home") != std::string::npos) {
                        if (build >= 26000) {
                            result = "Windows 11 Home 24H2";
                        } else {
                            result = "Windows 11 Home";
                        }
                    } else {
                        if (build >= 26000) {
                            result = "Windows 11 24H2";
                        } else {
                            result = "Windows 11";
                        }
                    }
                }
                // Add build number
                result += " (Build " + std::string(buildNumber) + ")";
                return result;
            }
        }
        
        // If we have product name but no Windows 11 detection needed
        if (hasProductName) {
            if (hasBuildNumber) {
                result += " (Build " + std::string(buildNumber) + ")";
            }
            return result;
        }
    }
    
    // Method 2: Fallback using RtlGetVersion
    HMODULE hMod = GetModuleHandleW(L"kernel32.dll");
    if (hMod) {
        typedef NTSTATUS (WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);
        RtlGetVersionPtr RtlGetVersion = (RtlGetVersionPtr)GetProcAddress(hMod, "RtlGetVersion");
        
        if (RtlGetVersion) {
            RTL_OSVERSIONINFOW osvi = { 0 };
            osvi.dwOSVersionInfoSize = sizeof(osvi);
            
            if (RtlGetVersion(&osvi) == 0) {
                std::ostringstream oss;
                
                // Enhanced Windows 11 detection
                if (osvi.dwMajorVersion == 10 && osvi.dwBuildNumber >= 22000) {
                    if (osvi.dwBuildNumber >= 26000) {
                        oss << "Windows 11 24H2 (Build " << osvi.dwBuildNumber << ")";
                    } else if (osvi.dwBuildNumber >= 22621) {
                        oss << "Windows 11 22H2 (Build " << osvi.dwBuildNumber << ")";
                    } else {
                        oss << "Windows 11 (Build " << osvi.dwBuildNumber << ")";
                    }
                } else if (osvi.dwMajorVersion == 10) {
                    oss << "Windows 10 (Build " << osvi.dwBuildNumber << ")";
                } else {
                    oss << "Windows " << osvi.dwMajorVersion << "." << osvi.dwMinorVersion 
                        << " (Build " << osvi.dwBuildNumber << ")";
                }
                
                return oss.str();
            }
        }
    }
    
    return "Windows (version unknown)";
#else
    struct utsname unameData;
    if (uname(&unameData) == 0) {
        std::ostringstream oss;
        oss << unameData.sysname << " " << unameData.release;
        return oss.str();
    }
    return "Unix-like";
#endif
}

std::string SystemInfo::toJson(const std::map<std::string, std::string>& info) {
    std::ostringstream json;
    json << "{";
    
    bool first = true;
    for (const auto& pair : info) {
        if (!first) json << ",";
        json << "\"" << pair.first << "\":\"" << pair.second << "\"";
        first = false;
    }
    
    json << "}";
    return json.str();
}