#pragma once

#include <string>
#include <vector>

#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <iostream>

class ProcessLister {
public:
    ProcessLister() = default;
    ~ProcessLister() = default;
    ProcessLister(const ProcessLister&) = default;
    ProcessLister& operator=(const ProcessLister&) = default;

    bool listProcesses() const;
    std::vector<std::string> getExeList() const;

private:
    static const std::string command;
};
