#include "ClientApp.hpp"
#include <iostream>

int main()
{
    try
    {
        ClientApp app;
        app.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception Client: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
