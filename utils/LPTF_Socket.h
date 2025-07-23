#ifndef LPTF_SOCKET_H
#define LPTF_SOCKET_H


#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
typedef int ssize_t;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include <stdexcept>
#include <string>
#include <memory>
using namespace std;

class LPTF_Socket {
private:
    int sockfd;
    sockaddr_in address;
    struct sockaddr_in clientAddress;
    void setupAddress(int port);

public:
    LPTF_Socket();  // main constructor
    LPTF_Socket(int sockfd, struct sockaddr_in clientAddr);  // constructor used by accept()
    ~LPTF_Socket();

    static void initialize();
    // GETTERS & SETTERS //
    int getSocketFd() const;

    void bindSocket(int port);
    void listenSocket();
    unique_ptr<LPTF_Socket> acceptSocket();

    void connectSocket(const string& ip, int port);

    ssize_t sendMsg(const string& msg);
    string recvMsg();
    string getClientIP();


};

#endif
