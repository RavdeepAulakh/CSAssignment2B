// Manages the actual communication with the Tomcat Upload Servlet

// uses the socket wrapper

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "FileUploadException.hpp"
#include "Socket.hpp"

using namespace std;

class UploadClient {
public:
    static void postFile(const string& serverAddress, int port, const string& caption, const string& date, const string& filePath) {
        ifstream fileStream(filePath, ios::in | ios::binary);

        if (!fileStream) {
            throw FileUploadException("File does not exist at the specified path.");
        }

        string fileContents((istreambuf_iterator<char>(fileStream)), istreambuf_iterator<char>()); // Read file contents

        Socket socket;
        socket.connectToServer(serverAddress, port);

        // Prepare the HTTP request with multipart form data
        string boundary = "**********BOUNDARY**********";
        string request = "";
        if(port == 8082){
            request = "POST /upload HTTP/1.1\r\n";
        } else if (port == 8081){
            request = "POST /upload/upload HTTP/1.1\r\n";
        }
        request += "Host: " + serverAddress + "\r\n";
        request += "Content-Type: multipart/form-data; boundary=" + boundary + "\r\n";
        request += "\r\n";

        request += "--" + boundary + "\r\n";
        request += "Content-Disposition: form-data; name=\"caption\"\r\n\r\n" + caption + "\r\n";

        request += "--" + boundary + "\r\n";
        request += "Content-Disposition: form-data; name=\"date\"\r\n\r\n" + date + "\r\n";

        request += "--" + boundary + "\r\n";
        if(port == 8082){
            request += "Content-Disposition: form-data; name=\"file\"; filename=\"" + filePath + "\"\r\n";
        } else if (port == 8081){
            request += "Content-Disposition: form-data; name=\"File\"; filename=\"" + filePath + "\"\r\n";
        }
        request += "Content-Type: application/octet-stream\r\n\r\n";
        request += fileContents + "\r\n";

        request += "--" + boundary + "--\r\n";

        socket.sendData(request);

        // Handle the server response (not shown in this simplified example)
    }
};

int main() {
    const string serverAddress = "localhost";
    const int port = 8082;
    const string filePath = "/Users/ravdeepaulakh/Documents/test/image.png";

    try {
        UploadClient::postFile(serverAddress, port, "Orange Truck", "2023-10-30", filePath);
        cout << "File uploaded successfully!" << endl;
    } catch (const FileUploadException& e) {
        cerr << "Error. Exception: " << e.what() << endl;
    }

    return 0;
}

