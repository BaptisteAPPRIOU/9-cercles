#ifndef LPTF_SOCKET_HPP
#define LPTF_SOCKET_HPP

#ifndef NOMINMAX
#define NOMINMAX
#endif

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
#include <vector>
#include <cstdint>
#include <stdexcept>

class LPTF_Socket {
private:
    int sockfd;
    sockaddr_in address;
    struct sockaddr_in clientAddress;
    void setupAddress(int port);

public:
    LPTF_Socket();
    LPTF_Socket(int sockfd, struct sockaddr_in clientAddr);
    ~LPTF_Socket();

    static void initialize();

    int getSocketFd() const;
    void bindSocket(int port);
    void listenSocket();
    std::unique_ptr<LPTF_Socket> acceptSocket();
    void connectSocket(const std::string& ip, int port);
    ssize_t sendMsg(const std::string& msg);
    std::string recvMsg();
    std::string getClientIP();
    ssize_t sendBinary(const std::vector<uint8_t>& data);
    std::vector<uint8_t> recvBinary();
    void closeSocket();
};

#endif // LPTF_SOCKET_HPP
