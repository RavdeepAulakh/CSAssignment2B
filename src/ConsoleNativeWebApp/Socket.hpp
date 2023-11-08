#ifndef SOCKET_H
#define SOCKET_H

#include <string>
#include <unistd.h>
#include <vector>
#include <sys/socket.h>

using namespace std;
class Socket {

private:

    int socket_fd;

public:

    Socket();


    bool connectToServer(const std::string& serverAddress, int port);

    bool sendData(const vector<char>& data);
};

#endif