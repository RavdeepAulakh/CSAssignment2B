//
// Created by Gathrean Dela Cruz on 2023-11-03.
//

#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <regex>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

class UploadServlet {
private:
    // The port number to listen for incoming connections
    const int PORT = 8081;

    // The socket file descriptor
    const std::string DIRECTORY_PATH = "./images/";

public:
    // Constructor
    UploadServlet();

    // Destructor
    ~UploadServlet();

    // Handles the request from the client
    void handlePOST(int clientSocket);

    // Handles the GET request from the client
    void handleGET(int clientSocket);

    // Starts the server
    void startServer();
};