#ifndef LPTF_SOCKET_H
#define LPTF_SOCKET_H

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include <string>
#include <memory>

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
    std::unique_ptr<LPTF_Socket> acceptSocket();

    void connectSocket(const std::string& ip, int port);

    ssize_t sendMsg(const std::string& msg);
    std::string recvMsg();
    std::string getClientIP();

    void closeSocket();
};

#endif
