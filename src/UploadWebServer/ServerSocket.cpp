#include "ServerSocket.hpp"
#include <stdexcept>
#include <cstring>
#include <unistd.h>

ServerSocket::ServerSocket(int port) : port(port), sockfd(-1) {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        throw std::runtime_error("Failed to open socket");
    }

    // Initialize server address structure
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
}

ServerSocket::~ServerSocket() {
    if (sockfd >= 0) {
        close(sockfd);
    }
}

void ServerSocket::Bind() {
    if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        throw std::runtime_error("Binding failed");
    }
}

void ServerSocket::Listen(int backlog) {
    if (listen(sockfd, backlog) < 0) {
        throw std::runtime_error("Error on listening");
    }
}

int ServerSocket::Accept() {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    int newsockfd = accept(sockfd, (struct sockaddr *) &client_addr, &client_len);
    if (newsockfd < 0) {
        throw std::runtime_error("Error on accept");
    }

    return newsockfd;
}


