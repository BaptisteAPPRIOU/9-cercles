#ifndef INETWORKINFO_HPP
#define INETWORKINFO_HPP

#include <string>
#include <vector>

/**
 * @class INetworkInfo
 * @brief Interface for retrieving network information (IP and MAC addresses).
 */
class INetworkInfo
{
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~INetworkInfo() = default;

    /**
     * @brief Gets all local IP addresses.
     * @return Vector of IP address strings.
     */
    virtual std::vector<std::string> getIPAddresses() const = 0;

    /**
     * @brief Gets all MAC addresses of network interfaces.
     * @return Vector of MAC address strings.
     */
    virtual std::vector<std::string> getMACAddresses() const = 0;

    /**
     * @brief Gets active (up) IP addresses.
     * @return Vector of active IP address strings.
     */
    virtual std::vector<std::string> getActiveIPAddresses() const = 0;

    /**
     * @brief Gets active (up) MAC addresses.
     * @return Vector of active MAC address strings.
     */
    virtual std::vector<std::string> getActiveMACAddresses() const = 0;
};
#endif
