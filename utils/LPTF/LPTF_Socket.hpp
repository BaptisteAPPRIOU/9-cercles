#pragma once

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

/**
 * @class LPTF_Socket
 * @brief Cross-platform TCP socket wrapper for the LPTF protocol.
 *
 * Provides methods for socket creation, connection, data transmission, and reception.
 */
class LPTF_Socket
{
public:
    /**
     * @brief Default constructor. Creates a new socket.
     */
    LPTF_Socket();

    /**
     * @brief Constructs a socket from an existing file descriptor and client address.
     * @param sockfd Socket file descriptor.
     * @param clientAddr Client address structure.
     */
    LPTF_Socket(int sockfd, struct sockaddr_in clientAddr);

    // Coplien : non copiable, mais déplaçable (socket unique)

    /**
     * @brief Deleted copy constructor (socket is unique).
     */
    LPTF_Socket(const LPTF_Socket &) = delete;

    /**
     * @brief Deleted copy assignment operator (socket is unique).
     */
    LPTF_Socket &operator=(const LPTF_Socket &) = delete;

    /**
     * @brief Move constructor.
     * @param other Socket to move from.
     */
    LPTF_Socket(LPTF_Socket &&other) noexcept;

    /**
     * @brief Move assignment operator.
     * @param other Socket to move from.
     * @return Reference to this socket.
     */
    LPTF_Socket &operator=(LPTF_Socket &&other) noexcept;

    /**
     * @brief Destructor. Closes the socket if open.
     */
    ~LPTF_Socket();

    /**
     * @brief Initializes the socket library (required on Windows).
     */
    static void initialize();

    /**
     * @brief Gets the socket file descriptor.
     * @return Socket file descriptor.
     */
    int getSocketFd() const;

    /**
     * @brief Binds the socket to the specified port.
     * @param port Port number to bind to.
     */
    void bindSocket(int port);

    /**
     * @brief Puts the socket into listening mode.
     */
    void listenSocket();

    /**
     * @brief Accepts an incoming connection.
     * @return Unique pointer to a new LPTF_Socket for the accepted connection.
     */
    std::unique_ptr<LPTF_Socket> acceptSocket();

    /**
     * @brief Connects the socket to a remote server.
     * @param ip IP address of the server.
     * @param port Port number of the server.
     */
    void connectSocket(const std::string &ip, int port);

    /**
     * @brief Sends a string message over the socket.
     * @param msg Message to send.
     * @return Number of bytes sent, or -1 on error.
     */
    ssize_t sendMsg(const std::string &msg) const;

    /**
     * @brief Receives a string message from the socket.
     * @return Received message as a string.
     */
    std::string recvMsg() const;

    /**
     * @brief Gets the IP address of the connected client.
     * @return Client IP address as a string.
     */
    std::string getClientIP() const;

    /**
     * @brief Sends binary data over the socket.
     * @param data Vector of bytes to send.
     * @return Number of bytes sent, or -1 on error.
     */
    ssize_t sendBinary(const std::vector<uint8_t> &data) const;

    /**
     * @brief Receives binary data from the socket.
     * @return Vector of received bytes.
     */
    std::vector<uint8_t> recvBinary() const;

    /**
     * @brief Closes the socket.
     */
    void closeSocket();

private:
    int sockfd;                ///< Socket file descriptor.
    sockaddr_in address;       ///< Socket address structure.
    sockaddr_in clientAddress; ///< Client address structure.

    /**
     * @brief Sets up the socket address structure for the given port.
     * @param port Port number to use.
     */
    void setupAddress(int port);
};
