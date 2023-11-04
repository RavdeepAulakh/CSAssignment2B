#include "Socket.hpp"
#include "FileUploadException.hpp"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cstring>

Socket::Socket() {
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        throw FileUploadException("Socket creation failed.");
    }
}

void Socket::connectToServer(const std::string &serverAddress, int port) {

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    struct addrinfo hints, *server_info;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;  // IPv4
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(serverAddress.c_str(), nullptr, &hints, &server_info) != 0) {
        // Print the error message when getaddrinfo fails
        throw FileUploadException("Invalid server address: " + serverAddress);
    }

    server.sin_addr = ((struct sockaddr_in*)server_info->ai_addr)->sin_addr;
    freeaddrinfo(server_info);

    if (connect(socket_fd, (struct sockaddr*)&server, sizeof(server)) < 0) {
        // Print the error message when the connection fails
        throw FileUploadException("Connection to the server failed.");
    }
}