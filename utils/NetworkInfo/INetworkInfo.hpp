#ifndef INETWORKINFO_HPP
#define INETWORKINFO_HPP

#include <string>
#include <vector>
#include <iomanip>

class INetworkInfo {
public:
    virtual ~INetworkInfo() = default;
    virtual std::vector<std::string> getIPAddresses() const = 0;
    virtual std::vector<std::string> getMACAddresses() const = 0;
    virtual std::vector<std::string> getActiveIPAddresses() const = 0;
    virtual std::vector<std::string> getActiveMACAddresses() const = 0;
};
#endif
