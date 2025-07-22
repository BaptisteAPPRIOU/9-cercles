// NetworkInfo_Windows.hpp
#ifndef NETWORKINFO_WINDOWS_HPP
#define NETWORKINFO_WINDOWS_HPP

#include "../utils/INetworkInfo.hpp"

#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
#include <sstream>

class NetworkInfo_Windows : public INetworkInfo {
public:
    std::vector<std::string> getIPAddresses() const override;
    std::vector<std::string> getMACAddresses() const override;
    std::vector<std::string> getActiveIPAddresses() const override;
    std::vector<std::string> getActiveMACAddresses() const override ;
};

#endif
