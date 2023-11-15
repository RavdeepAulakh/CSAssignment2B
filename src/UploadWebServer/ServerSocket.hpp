#ifndef SERVERSOCKET_HPP
#define SERVERSOCKET_HPP

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>

class ServerSocket {
private:
    int sockfd; // Socket file descriptor
    int port; // Port to listen on
    struct sockaddr_in server_addr; // Server address structure

public:
    ServerSocket(int port);
    ~ServerSocket();

    void Bind();
    void Listen(int backlog = 5);
    int Accept();
};

#endif // SERVERSOCKET_HPP