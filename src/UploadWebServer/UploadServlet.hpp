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
public:

    /*
     * Constructor
     */
    UploadServlet();

    /*
     * Destructor
     */
    ~UploadServlet();

    /*
     * Handles the POST request from the client
     */
    void handlePOST(int clientSocket);

    /*
     * Handles the GET request from the client
     */
     void handleGET(int clientSocket);

    const int PORT = 8081;
    const std::string DIRECTORY_PATH = "./images/";
};