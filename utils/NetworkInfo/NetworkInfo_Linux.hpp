// NetworkInfo_Linux.hpp
#pragma once

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


/**
 * @class NetworkInfo_Linux
 * @brief Provides network information for Linux systems.
 */
class NetworkInfo_Linux : public INetworkInfo {
public:
    NetworkInfo_Linux() = default;
    NetworkInfo_Linux(const NetworkInfo_Linux&) = default;
    NetworkInfo_Linux& operator=(const NetworkInfo_Linux&) = default;
    NetworkInfo_Linux(NetworkInfo_Linux&&) noexcept = default;
    NetworkInfo_Linux& operator=(NetworkInfo_Linux&&) noexcept = default;
    ~NetworkInfo_Linux() override = default;

    /**
     * @brief Gets all local IP addresses.
     * @return Vector of IP address strings.
     */
    std::vector<std::string> getIPAddresses() const override;

    /**
     * @brief Gets all MAC addresses of network interfaces.
     * @return Vector of MAC address strings.
     */
    std::vector<std::string> getMACAddresses() const override;

    /**
     * @brief Gets active (up) IP addresses.
     * @return Vector of active IP address strings.
     */
    std::vector<std::string> getActiveIPAddresses() const override;

    /**
     * @brief Gets active (up) MAC addresses.
     * @return Vector of active MAC address strings.
     */
    std::vector<std::string> getActiveMACAddresses() const override;
};


#endif // NETWORKINFO_LINUX_HPP