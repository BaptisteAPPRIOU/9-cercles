#include "LPTF_Socket.h"
#include <iostream>
#include "../utils/EnvLoader.h"
using namespace std;

int main() {
    try {
        LPTF_Socket::initialize();
        LPTF_Socket clientSocket;
        auto env = EnvLoader::loadEnv("../../.env");
        string ip = EnvLoader::loadIP("../../.env");
        int port = EnvLoader::loadPort("../../.env");

        clientSocket.connectSocket(ip, port);
        cout << "(Ecrire 'sortie' pour sortir)" << endl;
        while (true) {
            string msg;
            cout << "Entrez le message : ";
            getline(cin, msg);

            if (msg == "sortie") {// Exit the loop if the user types 'exit'
                break;
            }

            clientSocket.sendMsg(msg); // Send the user input to the server
            cout << clientSocket.recvMsg() << endl; // Receive and print the response from the server
        }

    } catch (const exception& e) {
        cerr << "Exception Client: " << e.what() << endl;
        return 1;
    }
    return 0;
}
