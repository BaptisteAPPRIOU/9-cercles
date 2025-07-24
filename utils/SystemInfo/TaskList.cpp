#include "TaskList.hpp"

// Executes the "tasklist" command and prints the list of running processes to the console.
// Returns true if the command executed successfully, false otherwise.
bool ProcessLister::listProcesses() const {
    std::cout << "Running tasklist command to retrieve list of running processes:" << std::endl;

    // Run the system command
    int returnValue = system(command.c_str());

    // Check if the command execution failed
    if (returnValue != 0) {
        std::cerr << "Error executing tasklist command" << std::endl;
        return false;
    }

    return true;
}

// Retrieves only the names of the currently running ".exe" processes.
// Returns a vector of strings containing the executable names.
std::vector<std::string> ProcessLister::getExeList() const {
    std::vector<std::string> exeList;

    // Open a pipe to read the output of the tasklist command
    FILE* pipe = _popen(command.c_str(), "r");
    if (!pipe) {
        std::cerr << "Failed to run tasklist." << std::endl;
        return exeList;
    }

    char buffer[512];
    bool headerSkipped = false;

    // Read the output of the command line by line
    while (fgets(buffer, sizeof(buffer), pipe)) {
        std::string line(buffer);

        // Skip the header lines of the tasklist output
        if (!headerSkipped) {
            if (line.find("====") != std::string::npos) {
                headerSkipped = true; // Start reading data after the header separator
            }
            continue; // Skip this line
        }

        // Extract the first word of the line (which is the executable name)
        std::istringstream iss(line);
        std::string exeName;
        iss >> exeName;

        // Only add names that end in ".exe"
        if (exeName.size() >= 4 &&
            exeName.substr(exeName.size() - 4) == ".exe") {
            exeList.push_back(exeName);
        }
    }

    // Close the pipe once reading is finished
    _pclose(pipe);

    return exeList;
}
