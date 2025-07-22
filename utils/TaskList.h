#ifndef PROCESS_LISTER_H
#define PROCESS_LISTER_H

#include <string>
#include <vector>

class ProcessLister {
public:
    bool listProcesses() const;
    std::vector<std::string> getExeList() const;

private:
    const std::string command = "tasklist";
};

#endif 
