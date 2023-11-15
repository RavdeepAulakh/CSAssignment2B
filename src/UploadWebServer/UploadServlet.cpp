#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <vector>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <algorithm>



const std::string DIRECTORY_PATH = "./images/";
const int PORT = 8083;
const int BUFFER_SIZE = 16384;



// Debugging utility to print to standard error
void debugLog(const std::string& message) {
    std::cerr << "DEBUG: " << message << std::endl;
}


// THIS WORKS
// Utility function to list files in the directory
std::string listFilesAsHTML(const std::string& directoryPath) {
    std::string fileListHTML = "<h2>Files in Directory</h2><ul>";
    DIR* dirp = opendir(directoryPath.c_str());
    struct dirent* dp;
    while ((dp = readdir(dirp)) != NULL) {
        if (dp->d_type == DT_REG) { // Only list regular files
            fileListHTML += "<li>" + std::string(dp->d_name) + "</li>";
        }
    }
    closedir(dirp);
    fileListHTML += "</ul>";
    return fileListHTML;
}


// Utility function to write a string to a socket
void writeStringToSocket(int socket, const std::string& string) {
    send(socket, string.c_str(), string.size(), 0);
}




// Function to receive and parse HTTP request
std::vector<std::string> receiveAndParseHttpRequest(int clientSocket) {
    debugLog("Receiving HTTP request");
    std::vector<char> buffer(BUFFER_SIZE);
    std::vector<std::string> data;
    std::string cur_str;

    // Receive data
    ssize_t bytes_received = recv(clientSocket, buffer.data(), buffer.size(), 0);
    if (bytes_received < 0) {
        debugLog("Failed to receive data");
        return data;
    } else {
        debugLog("Received data from client");
    }

    // Parse data
    for (int i = 0; i < bytes_received; ++i) {
        if (buffer[i] == '\r' && buffer[i + 1] == '\n') {
            data.push_back(cur_str);
            cur_str = "";
            ++i; // Skip the next character
        } else {
            cur_str.push_back(buffer[i]);
        }
    }
    if (!cur_str.empty()) {
        data.push_back(cur_str); // Add the last part
    }

    // Logging the received data for debugging
    for (const auto& line : data) {
        debugLog("Received line: " + line);
    }

    return data;
}



// Function to handle the GET request
void handleGET(int clientSocket) {
    std::string filesListHTML = listFilesAsHTML(DIRECTORY_PATH);
    std::ostringstream html;
    html << "HTTP/1.1 200 OK\r\n"
         << "Content-Type: text/html\r\n"
         << "\r\n"
         << "<html><head><title>File Upload Form</title></head><body>"
         << "<h1>File Upload Form</h1>"
         << "<form action='/upload' method='post' enctype='multipart/form-data'>"
         << "<label for='file'>Select a file:</label>"
         << "<input type='file' name='file' id='file'><br>"
         << "<label for='caption'>Caption:</label>"
         << "<input type='text' name='caption' id='caption'><br>"
         << "<label for='date'>Date:</label>"
         << "<input type='date' name='date' id='date'><br>"
         << "<input type='submit' value='Upload'>"
         << "</form>"
         << filesListHTML
         << "</body></html>";

    writeStringToSocket(clientSocket, html.str());
}


// Helper function to determine if a string starts with a given substring
bool startsWith(const std::string& fullString, const std::string& starting) {
    return fullString.compare(0, starting.size(), starting) == 0;
}


std::string sanitizeFilename(const std::string& filename) {
    std::string result;
    for (char ch : filename) {
        // Allow only alphanumeric characters, hyphens, underscores, and a dot for the extension
        if (isalnum(ch) || ch == '.' || ch == '-' || ch == '_') {
            result += ch;
        }
    }
    return result;
}



// Helper function to extract the boundary from the content type header
std::string extractBoundary(const std::string& contentType) {
    std::string boundaryPrefix = "boundary=";
    size_t boundaryPos = contentType.find(boundaryPrefix);
    if (boundaryPos != std::string::npos) {
        boundaryPos += boundaryPrefix.length();
        size_t endPos = contentType.length();
        if (contentType[boundaryPos] == '"') {
            boundaryPos++;
            endPos = contentType.find('"', boundaryPos);
        }
        if (endPos == std::string::npos) {
            endPos = contentType.length();
        }
        // The actual boundary is prefixed with "--"
        return "--" + contentType.substr(boundaryPos, endPos - boundaryPos);
    }
    return "";
}


// Function to handle POST request and save uploaded file
void handlePostRequest(int clientSocket) {
    debugLog("Entered handlePostRequest");

    std::vector<std::string> requestData = receiveAndParseHttpRequest(clientSocket);
    debugLog("Request data received. Size: " + std::to_string(requestData.size()));

    if (requestData.empty()) {
        debugLog("Error: Request data is empty");
        writeStringToSocket(clientSocket, "HTTP/1.1 400 Bad Request\r\n\r\n");
        return;
    }

    debugLog("Checking request method: " + requestData[0]);

    // Check the first line for the POST method
    if (!startsWith(requestData[0], "POST ")) {
        debugLog("Error: First line of request data does not start with POST");
        writeStringToSocket(clientSocket, "HTTP/1.1 400 Bad Request\r\n\r\n");
        return;
    }

    debugLog("Processing POST request");


    // Find and extract the boundary from the Content-Type header
    std::string boundary = extractBoundary(requestData[1]); // Assuming second line contains Content-Type
    debugLog("Boundary extracted: " + boundary);

    if (boundary.empty()) {
        debugLog("Error: Boundary not found in Content-Type header");
        writeStringToSocket(clientSocket, "HTTP/1.1 400 Bad Request\r\n\r\n");
        return;
    }

    // Process the multipart/form-data
    auto dataIter = requestData.begin();
    while (dataIter != requestData.end()) {
        debugLog("Processing part of the request data");

        // Find the boundary line and move to the next line (headers start here)
        dataIter = std::find(dataIter, requestData.end(), boundary);
        if (dataIter == requestData.end()) {
            debugLog("Boundary not found in request data");
            break;
        }
        ++dataIter; // Move past the boundary line

        if (dataIter == requestData.end()) {
            debugLog("End of request data reached after boundary");
            break;
        }

        // Initialize field name and file name
        std::string fieldName;
        std::string filename;

        // Parse headers
        while (dataIter != requestData.end() && !dataIter->empty()) {
            debugLog("Parsing header: " + *dataIter);
            if (startsWith(*dataIter, "Content-Disposition:")) {
                // Parse Content-Disposition header to get field name and file name
                size_t namePos = dataIter->find("name=\"");
                size_t filenamePos = dataIter->find("filename=\"");

                if (namePos != std::string::npos) {
                    namePos += 6; // Length of 'name="'
                    size_t endPos = dataIter->find("\"", namePos);
                    fieldName = dataIter->substr(namePos, endPos - namePos);
                    debugLog("Field name parsed: " + fieldName);
                }

                if (filenamePos != std::string::npos) {
                    filenamePos += 10; // Length of 'filename="'
                    size_t endPos = dataIter->find("\"", filenamePos);
                    filename = dataIter->substr(filenamePos, endPos - filenamePos);
                    debugLog("File name parsed: " + filename);
                }
            }
            ++dataIter;
        }

        // Skip the empty line after headers
        if (dataIter != requestData.end() && dataIter->empty()) {
            ++dataIter;
        }

        // Read the content until the next boundary
        std::vector<char> fileContent;
        while (dataIter != requestData.end() && !startsWith(*dataIter, boundary)) {
            fileContent.insert(fileContent.end(), dataIter->begin(), dataIter->end());
            ++dataIter;
        }

        if (!fileContent.empty() && fileContent.back() == '\n') {
            fileContent.pop_back(); // Remove last '\n'
            if (!fileContent.empty() && fileContent.back() == '\r') {
                fileContent.pop_back(); // Remove last '\r'
            }
        }

        // Save the content to a file if a filename is present
        if (!filename.empty()) {
            filename = sanitizeFilename(filename);
            std::ofstream outFile(DIRECTORY_PATH + filename, std::ios::binary);
            if (outFile.is_open()) {
                debugLog("Saving file: " + filename);
                outFile.write(fileContent.data(), fileContent.size());
                outFile.close();
            } else {
                debugLog("Failed to save file: " + filename);
            }
        }
    }

    debugLog("POST handling complete, sending 200 OK response");
    writeStringToSocket(clientSocket, "HTTP/1.1 200 OK\r\n\r\n");
}




// Main server function
void startServer() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Socket creation failed\n";
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, reinterpret_cast<struct sockaddr *>(&serverAddr), sizeof(serverAddr)) == -1) {
        std::cerr << "Binding failed\n";
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    if (listen(serverSocket, 10) == -1) {
        std::cerr << "Listening failed\n";
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    std::cout << "Server started on port " << PORT << std::endl;

    // Main server loop
    while (true) {
        int clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket == -1) {
            std::cerr << "Accept failed\n";
            continue;
        }

        std::vector<std::string> requestData = receiveAndParseHttpRequest(clientSocket);

        if (!requestData.empty() && startsWith(requestData[0], "GET ")) {
            handleGET(clientSocket);
        } else if (!requestData.empty() && startsWith(requestData[0], "POST /upload")) {
            handlePostRequest(clientSocket);
        } else {
            writeStringToSocket(clientSocket, "HTTP/1.1 400 Bad Request\r\n\r\n");
        }

        close(clientSocket);
    }


    // Close server socket, cleanup, etc.
    close(serverSocket);
}

int main() {
    debugLog("Starting server");
    // Ensure the images directory exists
    struct stat st = {0};
    if (stat(DIRECTORY_PATH.c_str(), &st) == -1) {
        mkdir(DIRECTORY_PATH.c_str(), 0700);
    }

    startServer();
    debugLog("Server shutdown");
    return 0;
}
