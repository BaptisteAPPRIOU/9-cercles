#include "NetworkInfo_Linux.hpp"

#if defined(__linux__) || (defined(__unix__) && !defined(__APPLE__))

namespace
{
    bool isPhysicalIface(const std::string &name)
    {
        return !(name == "lo" ||
                 name.find("docker") == 0 ||
                 name.find("veth") == 0 ||
                 name.find("vmnet") == 0 ||
                 name.find("virbr") == 0 ||
                 name.find("br-") == 0);
    }
}

std::vector<std::string> NetworkInfo_Linux::getIPAddresses() const
{
    std::vector<std::string> result;
    struct ifaddrs *ifaddr = nullptr;
    if (getifaddrs(&ifaddr) == -1)
        return result;
    for (struct ifaddrs *ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
    {
        if (!ifa->ifa_addr)
            continue;
        if (ifa->ifa_addr->sa_family == AF_INET)
        {
            char ip[INET_ADDRSTRLEN];
            void *addr_ptr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            inet_ntop(AF_INET, addr_ptr, ip, INET_ADDRSTRLEN);
            result.push_back(ip);
        }
    }
    freeifaddrs(ifaddr);
    return result;
}

std::vector<std::string> NetworkInfo_Linux::getMACAddresses() const
{
    std::vector<std::string> macs;
    struct ifaddrs *ifaddr = nullptr;
    if (getifaddrs(&ifaddr) == -1)
        return macs;
    for (struct ifaddrs *ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
    {
        if (!ifa->ifa_addr)
            continue;
        if (ifa->ifa_addr->sa_family == AF_PACKET)
        {
            struct sockaddr_ll *s = (struct sockaddr_ll *)ifa->ifa_addr;
            std::ostringstream oss;
            for (int i = 0; i < s->sll_halen; ++i)
            {
                oss << (i ? ":" : "") << std::hex << (s->sll_addr[i] & 0xff);
            }
            macs.push_back(oss.str());
        }
    }
    freeifaddrs(ifaddr);
    return macs;
}

std::vector<std::string> NetworkInfo_Linux::getActiveIPAddresses() const
{
    std::vector<std::string> ips;
    struct ifaddrs *ifaddr = nullptr;
    if (getifaddrs(&ifaddr) == -1)
        return ips;

    for (auto *ifa = ifaddr; ifa; ifa = ifa->ifa_next)
    {
        if (!ifa->ifa_addr)
            continue;
        if (ifa->ifa_addr->sa_family != AF_INET)
            continue;
        std::string name = ifa->ifa_name;
        if (!isPhysicalIface(name))
            continue;
        if (!(ifa->ifa_flags & IFF_UP))
            continue;

        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr, ip, sizeof(ip));
        ips.push_back(ip);
    }
    freeifaddrs(ifaddr);
    return ips;
}

std::vector<std::string> NetworkInfo_Linux::getActiveMACAddresses() const
{
    std::vector<std::string> macs;
    struct ifaddrs *ifaddr = nullptr;
    if (getifaddrs(&ifaddr) == -1)
        return macs;

    for (auto *ifa = ifaddr; ifa; ifa = ifa->ifa_next)
    {
        if (!ifa->ifa_addr)
            continue;
        if (ifa->ifa_addr->sa_family != AF_PACKET)
            continue;
        std::string name = ifa->ifa_name;
        if (!isPhysicalIface(name))
            continue;
        if (!(ifa->ifa_flags & IFF_UP))
            continue;

        struct sockaddr_ll *s = (struct sockaddr_ll *)ifa->ifa_addr;
        if (s->sll_halen == 6)
        {
            std::ostringstream oss;
            for (int i = 0; i < 6; ++i)
            {
                if (i)
                    oss << ":";
                oss << std::hex << std::uppercase << (s->sll_addr[i] & 0xff);
            }
            macs.push_back(oss.str());
        }
    }
    freeifaddrs(ifaddr);
    return macs;
}
#endif
