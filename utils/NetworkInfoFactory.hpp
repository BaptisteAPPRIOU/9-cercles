// NetworkInfoFactory.hpp
#ifndef NETWORKINFOFACTORY_HPP
#define NETWORKINFOFACTORY_HPP

#include "../utils/INetworkInfo.hpp"
#include <memory>

class NetworkInfoFactory {
public:
    static std::unique_ptr<INetworkInfo> create();
};

#endif
