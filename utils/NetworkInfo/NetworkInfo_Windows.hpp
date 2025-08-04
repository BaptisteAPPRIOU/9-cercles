// NetworkInfo_Windows.hpp
#ifndef NETWORKINFO_WINDOWS_HPP
#define NETWORKINFO_WINDOWS_HPP

#include "INetworkInfo.hpp"

#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
#include <sstream>


/**
 * @class NetworkInfo_Windows
 * @brief Provides network information for Windows systems.
 */
class NetworkInfo_Windows : public INetworkInfo {
public:
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

#endif
