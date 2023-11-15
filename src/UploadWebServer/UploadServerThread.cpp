// should inherit from given thread class in Tejinder's examples

// Upload server thread inherit from Thread class, to inherit run()
// see simple thread, but call it upload server thread

#include "UploadServerThread.hpp"
#include <iostream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include "HttpServletRequest.hpp"
#include "HttpServletResponse.hpp"
#include "UploadServlet.hpp"

using namespace std;


// Destructor
UploadServerThread::~UploadServerThread() {
    close(clientSocketFd);
}

// Static wrapper function to call HandleClient
void* UploadServerThread::ThreadFunc(void* arg) {
    UploadServerThread* uploadServerThread = static_cast<UploadServerThread*>(arg);
    uploadServerThread->HandleClient();
    return nullptr;
}

// HandleClient method
void UploadServerThread::HandleClient() {
    try {
        const int bufferSize = 4096;
        char buffer[bufferSize];
        stringstream requestStream;

        // Read data from the client
        ssize_t bytesRead;
        while ((bytesRead = read(clientSocketFd, buffer, bufferSize - 1)) > 0) {
            buffer[bytesRead] = '\0';
            requestStream.write(buffer, bytesRead);
            if (requestStream.str().find("\r\n\r\n") != string::npos) {
                break;
            }
        }

        if (bytesRead < 0) {
            throw runtime_error("Error reading from socket");
        }

        HttpServletRequest request(requestStream);
        stringstream responseStream;
        HttpServletResponse response(responseStream);

        UploadServlet servlet;
        string requestLine;
        getline(requestStream, requestLine);
        string method = requestLine.substr(0, requestLine.find(" "));

        if (method == "GET") {
            servlet.doGet(request, response);
        } else if (method == "POST") {
            servlet.doPost(request, response);
        }

        string responseStr = responseStream.str();
        write(clientSocketFd, responseStr.c_str(), responseStr.length());

    } catch (const exception &e) {
        cerr << "Error: " << e.what() << endl;
    }

    close(clientSocketFd);
}

// Start method
void UploadServerThread::Start() {
    int result = pthread_create(&thread, NULL, ThreadFunc, this);
    if (result != 0) {
        cerr << "Unable to create thread: " << strerror(result) << endl;
    }
    
}