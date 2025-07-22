// NetworkInfo_Windows.cpp
#include "../utils/NetworkInfo_Windows.hpp"

namespace
{
    // Filters: skip virtual adapters, loopback, etc.
    bool isPhysical(const IP_ADAPTER_ADDRESSES *aa)
    {
        // Physical Ethernet or Wi-Fi, and interface UP
        if (!(aa->OperStatus == IfOperStatusUp))
            return false;
        if (aa->IfType != IF_TYPE_ETHERNET_CSMACD && aa->IfType != IF_TYPE_IEEE80211)
            return false;
        std::wstring desc(aa->Description);
        if (desc.find(L"Virtual") != std::wstring::npos ||
            desc.find(L"VMware") != std::wstring::npos ||
            desc.find(L"Hyper-V") != std::wstring::npos ||
            desc.find(L"TAP") != std::wstring::npos ||
            desc.find(L"Loopback") != std::wstring::npos)
            return false;
        return true;
    }
}

// std::vector<std::string> NetworkInfo_Windows::getIPAddresses() const
// {
//     std::vector<std::string> ips;
//     char hostname[256];
//     if (gethostname(hostname, sizeof(hostname)) == SOCKET_ERROR)
//         return ips;
//     struct addrinfo hints = {};
//     hints.ai_family = AF_INET;
//     struct addrinfo *res = nullptr;
//     if (getaddrinfo(hostname, nullptr, &hints, &res) != 0)
//         return ips;
//     for (auto ptr = res; ptr; ptr = ptr->ai_next)
//     {
//         char ip[INET_ADDRSTRLEN];
//         inet_ntop(AF_INET, &((struct sockaddr_in *)ptr->ai_addr)->sin_addr, ip, sizeof(ip));
//         ips.push_back(ip);
//     }
//     freeaddrinfo(res);
//     return ips;
// }

std::vector<std::string> NetworkInfo_Windows::getIPAddresses() const {
    std::vector<std::string> ips;
    ULONG outBufLen = 15000;
    IP_ADAPTER_ADDRESSES* adapters = (IP_ADAPTER_ADDRESSES*)malloc(outBufLen);

    if (GetAdaptersAddresses(AF_INET, 0, nullptr, adapters, &outBufLen) == NO_ERROR) {
        for (auto* aa = adapters; aa; aa = aa->Next) {
            for (IP_ADAPTER_UNICAST_ADDRESS* ua = aa->FirstUnicastAddress; ua; ua = ua->Next) {
                if (ua->Address.lpSockaddr->sa_family == AF_INET) {
                    char ip[INET_ADDRSTRLEN];
                    sockaddr_in* sa = (sockaddr_in*)ua->Address.lpSockaddr;
                    inet_ntop(AF_INET, &(sa->sin_addr), ip, sizeof(ip));
                    ips.push_back(ip);
                }
            }
        }
    }
    free(adapters);
    return ips;
}

std::vector<std::string> NetworkInfo_Windows::getMACAddresses() const
{
    std::vector<std::string> macs;
    IP_ADAPTER_INFO adapterInfo[16];
    DWORD bufLen = sizeof(adapterInfo);
    if (GetAdaptersInfo(adapterInfo, &bufLen) == NO_ERROR)
    {
        PIP_ADAPTER_INFO pAdapterInfo = adapterInfo;
        while (pAdapterInfo)
        {
            std::ostringstream oss;
            for (UINT i = 0; i < pAdapterInfo->AddressLength; i++)
            {
                if (i)
                    oss << ":";
                oss << std::setw(2) << std::setfill('0') << std::uppercase << std::hex
                    << static_cast<int>(pAdapterInfo->Address[i]);
            }
            macs.push_back(oss.str());
            pAdapterInfo = pAdapterInfo->Next;
        }
    }
    return macs;
}

std::vector<std::string> NetworkInfo_Windows::getActiveIPAddresses() const
{
    std::vector<std::string> ips;
    ULONG outBufLen = 15000;
    IP_ADAPTER_ADDRESSES *adapters = (IP_ADAPTER_ADDRESSES *)malloc(outBufLen);

    if (GetAdaptersAddresses(AF_INET, 0, nullptr, adapters, &outBufLen) == NO_ERROR)
    {
        for (auto *aa = adapters; aa; aa = aa->Next)
        {
            if (!isPhysical(aa))
                continue;
            for (IP_ADAPTER_UNICAST_ADDRESS *ua = aa->FirstUnicastAddress; ua; ua = ua->Next)
            {
                if (ua->Address.lpSockaddr->sa_family == AF_INET)
                {
                    char ip[INET_ADDRSTRLEN];
                    sockaddr_in *sa = (sockaddr_in *)ua->Address.lpSockaddr;
                    inet_ntop(AF_INET, &(sa->sin_addr), ip, sizeof(ip));
                    ips.push_back(ip);
                }
            }
        }
    }
    free(adapters);
    return ips;
}

std::vector<std::string> NetworkInfo_Windows::getActiveMACAddresses() const
{
    std::vector<std::string> macs;
    ULONG outBufLen = 15000;
    IP_ADAPTER_ADDRESSES *adapters = (IP_ADAPTER_ADDRESSES *)malloc(outBufLen);

    if (GetAdaptersAddresses(AF_INET, 0, nullptr, adapters, &outBufLen) == NO_ERROR)
    {
        for (auto *aa = adapters; aa; aa = aa->Next)
        {
            if (!isPhysical(aa))
                continue;
            if (aa->PhysicalAddressLength == 6)
            {
                std::ostringstream oss;
                for (ULONG i = 0; i < 6; i++)
                {
                    if (i)
                        oss << ":";
                    oss << std::setw(2) << std::setfill('0') << std::uppercase << std::hex
                        << static_cast<int>(aa->PhysicalAddress[i]);
                }
                macs.push_back(oss.str());
            }
        }
    }
    free(adapters);
    return macs;
}