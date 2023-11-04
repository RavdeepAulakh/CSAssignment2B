// similar to our Java version

// contains the main method, it's the one we run

// use server socket class to create instance of server socket
// in the main program in Upload Server

#pragma once
#include "ServerSocket.hpp"
#include "UploadServerThread.hpp"
#include <stddef.h>

#include <iostream>

using namespace std;

// Entry point for the program
int main() {

  // Initialize ServerSocket pointer to null
  ServerSocket *serverSocket = nullptr;
  
  try {
    // Attempt to establish a new ServerSocket that listens to port 8082
    serverSocket = new ServerSocket(8083);

    // While the established connection exisits
      // An infinite while loop will continuously listen for new connections
    while (true) {
      
      // Create a UploadServerThread pointer to null;
      UploadServerThread *uploadServerThread = nullptr;

      try {

        // Create a ClientSocket pointer
          // This is used to make a new connection 
        Socket *clientSocket = serverSocket->Accept();

        // Create a new UploadServerThread to handle the new connection
        uploadServerThread = new UploadServerThread(clientSocket);

        // Start the UploadServerThread - handles client interaction
        uploadServerThread->Start();

      } catch (const exception& e){
        // If the ClientSocket connection or starting an UploadServerThread fails
          // Use 'cerr' to log the error message
        cerr << "Error: Could not establish an Upload Server Thread." << endl;
        cerr << "Error: " << e.what() << endl;
      }
    }
  } catch (const exception &e){
    // If the ServerSocket initialization failed
      // Use 'cerr' to log the error message
    cerr << "Error: Could not listen on port 8082" << endl;
    cerr << "Error:" << e.what() << endl;

    // Destroy/Deallocate the ServerSocket object
    delete serverSocket;
  }
  
  return 0;
};