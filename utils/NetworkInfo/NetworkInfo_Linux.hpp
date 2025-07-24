// NetworkInfo_Linux.hpp
#ifndef NETWORKINFO_LINUX_HPP
#define NETWORKINFO_LINUX_HPP
#include "INetworkInfo.hpp"
#if defined(__linux__) || (defined(__unix__) && !defined(__APPLE__))
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#endif
#include <cstring>
#include <sstream>

class NetworkInfo_Linux : public INetworkInfo {
public:
    std::vector<std::string> getIPAddresses() const override;
    std::vector<std::string> getMACAddresses() const override;
    std::vector<std::string> getActiveIPAddresses() const override;
    std::vector<std::string> getActiveMACAddresses() const override;
};


#endif // NETWORKINFO_LINUX_HPP