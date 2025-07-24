// NetworkInfoFactory.hpp
#ifndef NETWORKINFOFACTORY_HPP
#define NETWORKINFOFACTORY_HPP

#include "INetworkInfo.hpp"
#include <memory>

class NetworkInfoFactory {
public:
    static std::unique_ptr<INetworkInfo> create();
};

#endif
