#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include "FileUploadException.hpp"
#include "Socket.hpp"

using namespace std;

// Function to extract the filename from a file path (assuming a Windows path)
string extractFileName(const string& filePath) {
    size_t pos = filePath.find_last_of("\\/");
    return (pos != string::npos) ? filePath.substr(pos + 1) : filePath;
}

class UploadClient {
public:
    static void postFile(const string &serverAddress, int port, const string &caption, const string &date, const string &filePath) {
        string filename = extractFileName(filePath);

        ifstream fileStream(filePath, ios::binary | ios::ate);
        if (!fileStream) {
            cerr << "Error opening file!" << endl;
            return;
        }

        streamsize size = fileStream.tellg();
        fileStream.seekg(0, ios::beg);

        vector<char> buffer(size);
        if (!fileStream.read(buffer.data(), size)) {
            cerr << "Error reading file!" << endl;
            return;
        }

        Socket socket;
        if (!socket.connectToServer(serverAddress, port)) {
            cerr << "Error connecting to server!" << endl;
            return;
        }

        string boundary = "**********BOUNDARY**********";
        ostringstream headerStream;
        ostringstream nonBinaryStream;

        // Construct the non-binary part of the request
        nonBinaryStream << "--" << boundary << "\r\n";
        nonBinaryStream << "Content-Disposition: form-data; name=\"caption\"\r\n\r\n";
        nonBinaryStream << caption << "\r\n";
        nonBinaryStream << "--" << boundary << "\r\n";
        nonBinaryStream << "Content-Disposition: form-data; name=\"date\"\r\n\r\n";
        nonBinaryStream << date << "\r\n";
        nonBinaryStream << "--" << boundary << "\r\n";
        nonBinaryStream << "Content-Disposition: form-data; name=\"File\"; filename=\"" << filename << "\"\r\n";
        nonBinaryStream << "Content-Type: image/jpg\r\n\r\n";  // Indicate the content type of the file

// Calculate the sizes of the individual parts of the request
        string nonBinaryRequestPart = nonBinaryStream.str(); // Non-binary request part as string
        string closingBoundary = "\r\n--" + boundary + "--\r\n"; // Closing boundary string

        // Calculate the content length
        size_t nonBinaryRequestSize = nonBinaryRequestPart.size(); // Size of the non-binary request part
        size_t binaryDataSize = buffer.size(); // Size of the binary data
        size_t closingBoundarySize = closingBoundary.size(); // Size of the closing boundary

        size_t contentLength = nonBinaryRequestSize + binaryDataSize + closingBoundarySize;

        // Construct the HTTP headers
        headerStream << "POST /upload HTTP/1.1\r\n";
        headerStream << "Host: " << serverAddress << ":" << port << "\r\n"; // Include the port number
        headerStream << "Content-Type: multipart/form-data; boundary=" << boundary << "\r\n";
        headerStream << "Content-Length: " << contentLength << "\r\n"; // Correct content length
        headerStream << "User-Agent: UploadClientCpp/1.0\r\n";
        headerStream << "\r\n"; // Headers end

        // Convert stringstreams to strings
        string httpHeaders = headerStream.str();

        // Debug information
        ofstream debugFile("debug_output.txt");
        if (!debugFile) {
            cerr << "Error creating debug file!" << endl;
            return;
        }

        // Write headers and non-binary request to the debug file for inspection
        debugFile << "Headers:\n" << httpHeaders;
        debugFile << "Non-binary request:\n" << nonBinaryRequestPart;
        debugFile << "Binary data size: " << binaryDataSize << " bytes\n";
        debugFile << "Closing boundary size: " << closingBoundarySize << " bytes\n";
        debugFile << "Total expected content length: " << contentLength << " bytes\n";
        debugFile.close();

        // Send the headers, non-binary request part, binary data, and closing boundary
        socket.sendData(vector<char>(httpHeaders.begin(), httpHeaders.end()));
        socket.sendData(vector<char>(nonBinaryRequestPart.begin(), nonBinaryRequestPart.end()));
        socket.sendData(buffer);
        socket.sendData(vector<char>(closingBoundary.begin(), closingBoundary.end()));
    }
};

int main() {
    const string serverAddress = "localhost";
    const int port = 8082;
    const string filePath = "C:\\Users\\bardi\\OneDrive\\Pictures\\Lebron.jpg";

    try {
        UploadClient::postFile(serverAddress, port, "Orange Truck", "2023-10-30", filePath);
        cout << "File uploaded successfully!" << endl;
    } catch (const FileUploadException& e) {
        cerr << "Error. Exception: " << e.what() << endl;
    }

    return 0;
}
