#include "BashExec.hpp"

bool BashExec::executeToFile(const std::string &command, const std::string &filePath)
{
    std::ofstream outFile(filePath, std::ios::app); // APPEND mode
    if (!outFile.is_open())
        return false;

    std::array<char, 256> buffer;

#if defined(_WIN32)
    std::string fullCmd = "cmd /C " + command;
    FILE *pipe = _popen(fullCmd.c_str(), "r");
#else
    FILE *pipe = popen(command.c_str(), "r");
#endif

    if (!pipe)
    {
        outFile << "[ERROR] Pipe failed.\n";
        outFile.close();
        return false;
    }

    // Write the command to the output file
    outFile << "\n=== Résultat de la commande : " << command << " ===\n";

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
    {
        outFile << buffer.data();
        outFile.flush(); // Ensure data is written immediately
    }

#if defined(_WIN32)
    _pclose(pipe);
#else
    pclose(pipe);
#endif

    outFile << "\n=== Fin ===\n";

#ifdef _WIN32
    hideFile(filePath);
#endif

    outFile.close();
    return true;
}

// Hides the specified file (Windows only)
void BashExec::hideFile(const std::string &filePath)
{
#ifdef _WIN32
    SetFileAttributesA(filePath.c_str(), FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
#endif
}
