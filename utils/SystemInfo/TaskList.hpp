#pragma once

#include <string>
#include <vector>

#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <iostream>

/**
 * @class ProcessLister
 * @brief Provides methods to list running processes and retrieve executable names.
 */
class ProcessLister
{
public:
    /**
     * @brief Default constructor.
     */
    ProcessLister() = default;

    /**
     * @brief Destructor.
     */
    ~ProcessLister() = default;

    /**
     * @brief Copy constructor.
     * @param other The ProcessLister to copy from.
     */
    ProcessLister(const ProcessLister &) = default;

    /**
     * @brief Copy assignment operator.
     * @param other The ProcessLister to copy from.
     * @return Reference to this ProcessLister.
     */
    ProcessLister &operator=(const ProcessLister &) = default;

    /**
     * @brief Lists running processes using a system command.
     * @return True if the command executed successfully, false otherwise.
     */
    bool listProcesses() const;

    /**
     * @brief Gets a list of executable names of running processes.
     * @return Vector of executable names as strings.
     */
    std::vector<std::string> getExeList() const;

private:
    static const std::string command; ///< System command used to list processes.
};
