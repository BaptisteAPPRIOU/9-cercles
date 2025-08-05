// NetworkInfoFactory.hpp
#ifndef NETWORKINFOFACTORY_HPP
#define NETWORKINFOFACTORY_HPP

#include "INetworkInfo.hpp"
#include <memory>

/**
 * @class NetworkInfoFactory
 * @brief Factory for creating platform-specific INetworkInfo implementations.
 */
class NetworkInfoFactory
{
public:
    /**
     * @brief Creates a platform-specific INetworkInfo instance.
     * @return Unique pointer to an INetworkInfo implementation for the current platform.
     */
    static std::unique_ptr<INetworkInfo> create();
};

#endif
