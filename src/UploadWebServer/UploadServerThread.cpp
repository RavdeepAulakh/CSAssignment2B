// should inherit from given thread class in Tejinder's examples
// Upload server thread inherit from Thread class, to inherit run()
// see simple thread, but call it upload server thread

#pragma once
#include <iostream>
#include <pthread.h>
#include "Socket.hpp"

using namespace std;

class UploadServerThread {
private:
    int clientSocketFd;

    // Static wrapper function to call HandleClient
    static void *ThreadFunc(void *arg) {
        UploadServerThread *uploadServerThread = static_cast<UploadServerThread*>(arg);
        uploadServerThread->HandleClient();
        return nullptr;
    }

    void HandleClient() {
       try {
            const int bufferSize = 4096;
            char buffer[bufferSize];
            stringstream requestStream;

            // Read data from the client
            ssize_t bytesRead;
            while ((bytesRead = read(clientSocketFd, buffer, bufferSize - 1)) > 0) {
                buffer[bytesRead] = '\0';
                requestStream.write(buffer, bytesRead);
                // Assuming HTTP request ends with "\r\n\r\n"
                if (requestStream.str().find("\r\n\r\n") != string::npos) {
                    break;
                }
            }

            if (bytesRead < 0) {
                throw runtime_error("Error reading from socket");
            }

            // Create HttpServletRequest and HttpServletResponse
            HttpServletRequest request(requestStream);
            stringstream responseStream;
            HttpServletResponse response(responseStream);

            // Create an instance of UploadServlet and process the request
            UploadServlet servlet;
            // Assuming the first line of the request contains the HTTP method and URI
            string requestLine;
            getline(requestStream, requestLine);
            string method = requestLine.substr(0, requestLine.find(" "));
            string urlPath = requestLine.substr(requestLine.find(" ") + 1, requestLine.find(" ", requestLine.find(" ") + 1) - requestLine.find(" ") - 1);

            // Handle the request based on the method
            if (method == "GET") {
                servlet.doGet(request, response);
            } else if (method == "POST") {
                servlet.doPost(request, response);
            }

            // Write response back to the client
            string responseStr = responseStream.str();
            write(clientSocketFd, responseStr.c_str(), responseStr.length());

        } catch (const exception &e) {
            cerr << "Error: " << e.what() << endl;
        }

        // Close the client socket
        close(clientSocketFd);
    }

public:
    // Constructor
    UploadServerThread(int clientSocketFd) : clientSocketFd(clientSocketFd) {}

    // Function to start the thread
    void Start() {
        int result = pthread_create(&uploadThread, NULL, ThreadFunc, this);
        if (result != 0) {
            cerr << "Unable to create thread: " << strerror(result) << endl;
        }
    }

    // Destructor
    ~UploadServerThread() {
        if (clientSocket != nullptr) {
            delete clientSocket;
        }
    }
};
