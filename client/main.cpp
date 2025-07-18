#include "LPTF_Socket.h"
#include <iostream>

int main() {
    try {
        LPTF_Socket::initialize();
        LPTF_Socket clientSocket;
        clientSocket.connectSocket("127.0.0.1", 8888);
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
