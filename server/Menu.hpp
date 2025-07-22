#ifndef MENU_HPP
#define MENU_HPP

#include "../utils/LPTF_Socket.hpp"
#include "../utils/LPTF_Packet.hpp"
#include "../utils/TaskList.hpp"
#include <vector>
#include <memory>
#include <limits>
#include <iomanip>
#include <iostream>
// Prevent Windows min/max macros from colliding with std::numeric_limits and std::max
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

class Menu {
public:
    Menu(const Menu&) = delete;
    Menu& operator=(const Menu&) = delete;
    ~Menu() = default;
    explicit Menu(std::vector<std::unique_ptr<LPTF_Socket>>& clients);
    int run();
    void waitNext();

private:
    std::vector<std::unique_ptr<LPTF_Socket>>& clients;
    void showOptions();
    int getChoice();
    void handleChoice(int choice);
};

#endif // MENU_HPP
