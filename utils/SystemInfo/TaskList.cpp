#include "TaskList.hpp"

#ifdef _WIN32
const std::string ProcessLister::command = "tasklist";
#else
const std::string ProcessLister::command = "ps -e";
#endif

bool ProcessLister::listProcesses() const {
    std::cout << "Running command to retrieve list of running processes:" << std::endl;
    int returnValue = system(command.c_str());
    if (returnValue != 0) {
        std::cerr << "Error executing process list command" << std::endl;
        return false;
    }
    return true;
}

std::vector<std::string> ProcessLister::getExeList() const {
    std::vector<std::string> exeList;
#ifdef _WIN32
    FILE* pipe = _popen(command.c_str(), "r");
#else
    FILE* pipe = popen(command.c_str(), "r");
#endif
    if (!pipe) {
        std::cerr << "Failed to run process list command." << std::endl;
        return exeList;
    }

    char buffer[512];
    bool headerSkipped = false;

    while (fgets(buffer, sizeof(buffer), pipe)) {
        std::string line(buffer);

#ifdef _WIN32
        if (!headerSkipped) {
            if (line.find("====") != std::string::npos) {
                headerSkipped = true;
            }
            continue;
        }
        std::istringstream iss(line);
        std::string exeName;
        iss >> exeName;
        if (exeName.size() >= 4 &&
            exeName.substr(exeName.size() - 4) == ".exe") {
            exeList.push_back(exeName);
        }
#else
        // Linux: extract command name (field may differ per OS)
        if (headerSkipped || (headerSkipped = (line.find("PID") != std::string::npos))) continue;
        std::istringstream iss(line);
        std::string pid, cmd;
        iss >> pid >> cmd;
        if (!cmd.empty()) exeList.push_back(cmd);
#endif
    }
#ifdef _WIN32
    _pclose(pipe);
#else
    pclose(pipe);
#endif
    return exeList;
}
