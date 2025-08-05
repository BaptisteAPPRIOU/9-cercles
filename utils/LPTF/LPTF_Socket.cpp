#include "LPTF_Socket.hpp"
#include <algorithm>

LPTF_Socket::LPTF_Socket() : sockfd(socket(AF_INET, SOCK_STREAM, 0))
{
    if (sockfd < 0)
    {
        throw std::runtime_error("Échec de la création du socket");
    }
}

LPTF_Socket::LPTF_Socket(int fd, sockaddr_in clientAddr)
    : sockfd(fd), clientAddress(clientAddr) {}

LPTF_Socket::LPTF_Socket(LPTF_Socket &&other) noexcept
    : sockfd(other.sockfd), address(other.address), clientAddress(other.clientAddress)
{
    other.sockfd = -1;
}

LPTF_Socket &LPTF_Socket::operator=(LPTF_Socket &&other) noexcept
{
    if (this != &other)
    {
        closeSocket();
        sockfd = other.sockfd;
        address = other.address;
        clientAddress = other.clientAddress;
        other.sockfd = -1;
    }
    return *this;
}

LPTF_Socket::~LPTF_Socket()
{
    closeSocket();
}

void LPTF_Socket::initialize()
{
#ifdef _WIN32
    static bool initialized = false;
    if (!initialized)
    {
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0)
        {
            throw std::runtime_error("Échec de WSAStartup : " + std::to_string(result));
        }
        initialized = true;
    }
#endif
}

int LPTF_Socket::getSocketFd() const { return sockfd; }

void LPTF_Socket::setupAddress(int port)
{
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = INADDR_ANY;
}

void LPTF_Socket::bindSocket(int port)
{
    int opt = 1;
#ifdef _WIN32
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));
#else
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif
    setupAddress(port);
    if (bind(sockfd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        throw std::runtime_error("Échec de la liaison du socket");
    }
}

void LPTF_Socket::listenSocket()
{
    if (listen(sockfd, 5) < 0)
    {
        throw std::runtime_error("Échec de l'écoute du socket");
    }
}

std::unique_ptr<LPTF_Socket> LPTF_Socket::acceptSocket()
{
    sockaddr_in clientAddr{};
    socklen_t addrlen = sizeof(clientAddr);
    int new_socket = accept(sockfd, (struct sockaddr *)&clientAddr, &addrlen);
    if (new_socket < 0)
    {
        throw std::runtime_error("Échec de l'acceptation de la connexion");
    }
    return std::make_unique<LPTF_Socket>(new_socket, clientAddr);
}

void LPTF_Socket::connectSocket(const std::string &ip, int port)
{
    setupAddress(port);
    address.sin_addr.s_addr = inet_addr(ip.c_str());
    if (connect(sockfd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        throw std::runtime_error("Échec de la connexion au serveur");
    }
}

ssize_t LPTF_Socket::sendMsg(const std::string &message) const
{
    return send(sockfd, message.c_str(), message.size(), 0);
}

std::string LPTF_Socket::recvMsg() const
{
    char buffer[1024] = {0};
    ssize_t valread = recv(sockfd, buffer, 1024, 0);
    if (valread < 0)
    {
        throw std::runtime_error("Échec de la réception");
    }
    return std::string(buffer, valread);
}

std::string LPTF_Socket::getClientIP() const
{
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(clientAddress.sin_addr), client_ip, INET_ADDRSTRLEN);
    return std::string(client_ip);
}

ssize_t LPTF_Socket::sendBinary(const std::vector<uint8_t> &data) const
{
    const char *buffer = reinterpret_cast<const char *>(data.data());
    size_t totalSent = 0;
    while (totalSent < data.size())
    {
        ssize_t sent = send(sockfd, buffer + totalSent, data.size() - totalSent, 0);
        if (sent <= 0)
        {
            throw std::runtime_error("Erreur lors de l'envoi binaire");
        }
        totalSent += sent;
    }
    return totalSent;
}

std::vector<uint8_t> LPTF_Socket::recvBinary() const
{
    uint8_t header[11];
    ssize_t headerLen = recv(sockfd, reinterpret_cast<char *>(header), 11, MSG_WAITALL);
    if (headerLen != 11)
    {
        throw std::runtime_error("Erreur de lecture de l'en-tête du paquet");
    }
    uint16_t payloadSize = (static_cast<uint16_t>(header[9]) << 8) | header[10];
    std::vector<uint8_t> fullData(11 + payloadSize);
    std::copy(header, header + 11, fullData.begin());
    if (payloadSize > 0)
    {
        ssize_t payloadLen = recv(sockfd, reinterpret_cast<char *>(fullData.data() + 11), payloadSize, MSG_WAITALL);
        if (payloadLen != payloadSize)
        {
            throw std::runtime_error("Erreur de lecture du payload");
        }
    }
    return fullData;
}

void LPTF_Socket::closeSocket()
{
    if (sockfd != -1)
    {
#ifdef _WIN32
        closesocket(sockfd);
#else
        close(sockfd);
#endif
        sockfd = -1;
    }
}
