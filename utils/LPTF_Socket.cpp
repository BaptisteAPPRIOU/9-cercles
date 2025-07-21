#include "LPTF_Socket.h"

// Implementation of LPTF_Socket class methods
LPTF_Socket::LPTF_Socket() : sockfd(socket(AF_INET, SOCK_STREAM, 0)) {
    if (sockfd < 0) {
        throw runtime_error("Échec de la création du socket");
    }
}

LPTF_Socket::LPTF_Socket(int fd, sockaddr_in clientAddr)
    : sockfd(fd), clientAddress(clientAddr) {}

LPTF_Socket::~LPTF_Socket() {
    #ifdef _WIN32
        closesocket(sockfd);
    #else
        close(sockfd);
    #endif
}

// Initializes the socket library (Windows-specific)
void LPTF_Socket::initialize() {
#ifdef _WIN32
    static bool initialized = false;
    if (!initialized) {
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            throw runtime_error("Échec de WSAStartup : " + to_string(result));
        }
        initialized = true;
    }
#endif
}

// Returns the socket file descriptor
int LPTF_Socket::getSocketFd() const {
    return sockfd;
}

// Sets up the address structure for binding or connecting
void LPTF_Socket::setupAddress(int port) {
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = INADDR_ANY;
}

// Binds the socket to the specified port
void LPTF_Socket::bindSocket(int port) {
    setupAddress(port);
    if (bind(sockfd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        throw runtime_error("Échec de la liaison du socket");
    }
}

// Listens for incoming connections on the socket
void LPTF_Socket::listenSocket() {
    if (listen(sockfd, 5) < 0) {
        throw runtime_error("Échec de l'écoute du socket");
    }
}

// Accepts a new connection and returns a unique pointer to a new LPTF_Socket instance
unique_ptr<LPTF_Socket> LPTF_Socket::acceptSocket() {
    struct sockaddr_in clientAddr;
    socklen_t addrlen = sizeof(clientAddr);
    int new_socket = accept(sockfd, (struct sockaddr*)&clientAddr, &addrlen);
    if (new_socket < 0) {
        throw runtime_error("Échec de l'acceptation de la connexion");
    }
    return make_unique<LPTF_Socket>(new_socket, clientAddr);
}

// Connects the socket to a server at the specified IP and port
void LPTF_Socket::connectSocket(const string& ip, int port) {
    setupAddress(port);
    address.sin_addr.s_addr = inet_addr(ip.c_str());
    if (connect(sockfd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        throw runtime_error("Échec de la connexion au serveur");
    }
}

// Sends a message through the socket
ssize_t LPTF_Socket::sendMsg(const string& message) {
    return send(sockfd, message.c_str(), message.size(), 0);
}

// Receives a message from the socket
string LPTF_Socket::recvMsg() {
    char buffer[1024] = {0};
    ssize_t valread = recv(sockfd, buffer, 1024, 0);
    if (valread < 0) {
        throw runtime_error("Échec de la réception");
    }
    return string(buffer, valread);
}

// Returns the IP address of the connected client
string LPTF_Socket::getClientIP() {
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(this->clientAddress.sin_addr), client_ip, INET_ADDRSTRLEN);
    return string(client_ip);
}

ssize_t LPTF_Socket::sendBinary(const std::vector<uint8_t>& data) {
    const char* buffer = reinterpret_cast<const char*>(data.data());
    size_t totalSent = 0;
    while (totalSent < data.size()) {
        ssize_t sent = send(sockfd, buffer + totalSent, data.size() - totalSent, 0);
        if (sent <= 0) {
            throw std::runtime_error("Erreur lors de l'envoi binaire");
        }
        totalSent += sent;
    }
    return totalSent;
}

std::vector<uint8_t> LPTF_Socket::recvBinary() {
    uint8_t header[4];
    ssize_t headerLen = recv(sockfd, reinterpret_cast<char*>(header), 4, MSG_WAITALL);
    if (headerLen != 4) {
        throw std::runtime_error("Erreur de lecture de l'en-tête du paquet");
    }

    uint16_t payloadSize = (header[2] << 8) | header[3];
    std::vector<uint8_t> fullData(4 + payloadSize);
    std::copy(header, header + 4, fullData.begin());

    ssize_t payloadLen = recv(sockfd, reinterpret_cast<char*>(fullData.data() + 4), payloadSize, MSG_WAITALL);
    if (payloadLen != payloadSize) {
        throw std::runtime_error("Erreur de lecture du payload");
    }

    return fullData;
}

void LPTF_Socket::closeSocket() {
#ifdef _WIN32
    if (sockfd != -1) {
        closesocket(sockfd);
        sockfd = -1;
    }
#else
    if (sockfd != -1) {
        close(sockfd);
        sockfd = -1;
    }
#endif
}