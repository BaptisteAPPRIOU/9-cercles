#ifndef MENU_HPP
#define MENU_HPP

#include "LPTF_Socket.h"
#include "LPTF_Packet.h"
#include <vector>
#include <memory>
#include <limits>
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
    // Run the menu loop; returns the selected exit code
    int run();

private:
    std::vector<std::unique_ptr<LPTF_Socket>>& clients;
    void showOptions();
    int getChoice();
    void handleChoice(int choice);
};

#endif // MENU_HPP
