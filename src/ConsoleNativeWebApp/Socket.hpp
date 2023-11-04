#ifndef SOCKET_H
#define SOCKET_H

#include <string>
#include <unistd.h>
#include <sys/socket.h>

class Socket {

private:

    int socket_fd;

public:

    Socket();

    inline ~Socket() {
        close(socket_fd);
    }

    void connectToServer(const std::string& serverAddress, int port);

    inline void sendData(const std::string& data) {
        send(socket_fd, data.c_str(), data.size(), 0);
    };
};

#endif