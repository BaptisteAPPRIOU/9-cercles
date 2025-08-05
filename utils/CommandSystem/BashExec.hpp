#ifndef BASHEXEC_HPP
#define BASHEXEC_HPP

#include <string>
#include <fstream>
#include <cstdio>
#include <array>

#ifdef _WIN32
#include <windows.h>
#endif

class BashExec
{
public:
    // Executes a system command and writes the output to a hidden file.
    // Returns true if writing was successful.
    static bool executeToFile(const std::string &command, const std::string &filePath);

    // Hides the specified file (Windows only).
    static void hideFile(const std::string &filePath);
};

#endif // BASHEXEC_HPP
