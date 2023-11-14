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
#include "HttpServlet.hpp"

class UploadServlet : public HttpServlet {
private:
    // The port number to listen for incoming connections
    const int PORT = 8081;

    // The socket file descriptor
    const std::string DIRECTORY_PATH = "./images/";

    // Handles the request from the client
    void handlePOST(int clientSocket);

    // Handles the GET request from the client
    void handleGET(int clientSocket);

public:
    // Constructor
    UploadServlet();

    // Destructor
    virtual ~UploadServlet();

    void doGet(HttpServletRequest& request, HttpServletResponse& response) override;

    void doPost(HttpServletRequest& request, HttpServletResponse& response) override;

    // Starts the server
    void startServer();
};