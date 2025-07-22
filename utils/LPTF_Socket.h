#ifndef LPTF_SOCKET_H
#define LPTF_SOCKET_H

#ifndef NOMINMAX
#define NOMINMAX
#endif
#define byte win_byte_override

#ifdef _WIN32
// prevent Windows headers from defining min/max macros
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include <cstdint>
#include <stdexcept>
#include <algorithm>
#include <string>
#include <memory>
#include <vector>
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

    ssize_t sendBinary(const std::vector<uint8_t>& data);
    std::vector<uint8_t> recvBinary();

    void closeSocket();
};

#endif
