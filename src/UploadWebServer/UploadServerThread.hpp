#ifndef UPLOADSERVERTHREAD_H
#define UPLOADSERVERTHREAD_H

#include <pthread.h>
#include "ServerSocket.hpp" 

class UploadServerThread {
private:
    int clientSocketFd; // Client socket file descriptor
    pthread_t thread;   // Thread handling this specific client

    // Any other private members or helper methods

public:
    // Constructor
    UploadServerThread(int clientSocketFd) : clientSocketFd(clientSocketFd) {}

    // Destructor
    ~UploadServerThread();

    // Method to start the thread
    void Start();

    // Static method that serves as a thread entry point
    static void* ThreadFunc(void* arg);

    // The main method that handles client requests
    void HandleClient();
};

#endif // UPLOADSERVERTHREAD_H