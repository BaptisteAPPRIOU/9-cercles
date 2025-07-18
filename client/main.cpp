#include "LPTF_Socket.h"
#include <iostream>
#include "9-cercles\utils\EnvLoader.cpp"

int main() {
    try {
        LPTF_Socket::initialize();
        LPTF_Socket clientSocket;
        auto env = loadEnv(".env");
        std::string ip = env["IP"];
        int port = std::stoi(env["PORT"]);

        clientSocket.connectSocket(ip, port);
        std::cout << "(Ecrire 'sortie' pour sortir)" << std::endl;
        while (true) {
            std::string msg;
            std::cout << "Entrez le message : ";
            std::getline(std::cin, msg);

            if (msg == "sortie") {// Exit the loop if the user types 'exit'
                break;
            }

            clientSocket.sendMsg(msg); // Send the user input to the server
            std::cout << clientSocket.recvMsg() << std::endl; // Receive and print the response from the server
        }

    } catch (const std::exception& e) {
        std::cerr << "Exception Client: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
