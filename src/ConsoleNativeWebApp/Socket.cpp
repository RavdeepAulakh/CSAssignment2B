#include "Socket.hpp"
#include "FileUploadException.hpp"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cstring>
#include <iostream>

Socket::Socket() {
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        throw FileUploadException("Socket creation failed.");
    }
}

bool Socket::connectToServer(const std::string &serverAddress, int port) {
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    struct addrinfo hints, *server_info;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;  // IPv4
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(serverAddress.c_str(), nullptr, &hints, &server_info);
    if (status != 0) {
        std::cerr << "getaddrinfo error: " << gai_strerror(status) << std::endl;
        return false;
    }

    server.sin_addr = ((struct sockaddr_in*)server_info->ai_addr)->sin_addr;
    freeaddrinfo(server_info);

    if (connect(socket_fd, (struct sockaddr*)&server, sizeof(server)) < 0) {
        std::cerr << "Connection failed: " << strerror(errno) << std::endl;
        return false;
    }

    return true; // Connection was successful
}
bool Socket::sendData(const vector<char>& data) {
    size_t totalSent = 0;
    while (totalSent < data.size()) {
        ssize_t sent = send(socket_fd, data.data() + totalSent, data.size() - totalSent, 0);
        if (sent == -1) {
            // Handle error, e.g., by logging it and returning false
            return false;
        }
        totalSent += sent;
    }
    return true;
}

