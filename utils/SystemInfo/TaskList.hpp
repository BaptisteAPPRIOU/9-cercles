#ifndef PROCESS_LISTER_HPP
#define PROCESS_LISTER_HPP

#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <sstream> 
#include <iostream>
#include <algorithm>

class ProcessLister {
public:
    bool listProcesses() const;
    std::vector<std::string> getExeList() const;

private:
    const std::string command = "tasklist";
};

#endif // PROCESS_LISTER_HPP
