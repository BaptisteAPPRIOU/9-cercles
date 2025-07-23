#ifndef INETWORKINFO_HPP
#define INETWORKINFO_HPP

#include <string>
#include <vector>
#include <iomanip>

class INetworkInfo {
public:
    virtual ~INetworkInfo() = default;
    // Renvoie la liste de toutes les adresses IP locales
    virtual std::vector<std::string> getIPAddresses() const = 0;
    // Renvoie la liste de toutes les adresses MAC des interfaces réseau
    virtual std::vector<std::string> getMACAddresses() const = 0;
    virtual std::vector<std::string> getActiveIPAddresses() const = 0;
    virtual std::vector<std::string> getActiveMACAddresses() const = 0;
};
#endif
