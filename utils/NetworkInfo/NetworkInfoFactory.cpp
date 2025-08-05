// NetworkInfoFactory.cpp
#include "NetworkInfoFactory.hpp"
#if defined(_WIN32) || defined(_WIN64)
#include "NetworkInfo_Windows.hpp"
#else
#include "NetworkInfo_Linux.hpp"
#endif

std::unique_ptr<INetworkInfo> NetworkInfoFactory::create()
{
#if defined(_WIN32) || defined(_WIN64)
    return std::make_unique<NetworkInfo_Windows>();
#else
    return std::make_unique<NetworkInfo_Linux>();
#endif
}
