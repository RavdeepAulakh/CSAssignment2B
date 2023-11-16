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
void debugLog(const std::string &message) {
    std::cerr << "DEBUG: " << message << std::endl;
}


// THIS WORKS
// Utility function to list files in the directory
std::string listFilesAsHTML(const std::string &directoryPath) {
    std::string fileListHTML = "<h2>Files in Directory</h2><ul>";
    DIR *dirp = opendir(directoryPath.c_str());
    struct dirent *dp;
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
void writeStringToSocket(int socket, const std::string &string) {
    send(socket, string.c_str(), string.size(), 0);
}


// Function to receive and parse HTTP request
std::vector<std::string> receiveAndParseHttpRequest(int clientSocket) {
    debugLog("Receiving HTTP request");
    char buf;
    std::string cur_str;
    std::vector<std::string> data;
    int i = 0;
    char buf1[4] = {}; // Buffer to check for end of headers or payload

    while (true) {
        ssize_t rval = read(clientSocket, &buf, 1);
        if (rval <= 0) {
            // Handle read error or no data
            debugLog("Failed to receive data or connection closed");
            break;
        }

        // Check for end of headers
        buf1[i % 4] = buf;
        if (i >= 3 && buf1[i % 4] == '\n' && buf1[(i - 1) % 4] == '\r' &&
            buf1[(i - 2) % 4] == '\n' && buf1[(i - 3) % 4] == '\r') {
            // Headers end detected
            data.push_back(cur_str);
            break;
        }

        if (buf == '\r' || buf == '\n') {
            if (!cur_str.empty()) {
                data.push_back(cur_str);
                cur_str.clear();
            }
        } else {
            cur_str.push_back(buf);
        }

        i++;
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
bool startsWith(const std::string &fullString, const std::string &starting) {
    return fullString.compare(0, starting.size(), starting) == 0;
}


// Helper function to extract the boundary from the content type header
std::string extractBoundary(const std::string &contentType) {
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
void handlePostRequest(int clientSocket, std::vector<std::string> requestData) {
    debugLog("Entered handlePostRequest");

    if (requestData.empty()) {
        debugLog("Error: Request data is empty");
        writeStringToSocket(clientSocket, "HTTP/1.1 400 Bad Request\r\n\r\n");
        return;
    }

    debugLog("Starting to process request data");

    // Find the Content-Type header to get the boundary
    std::string boundary;
    for (const auto &line : requestData) {
        if (startsWith(line, "Content-Type:")) {
            debugLog("Found Content-Type header: " + line);
            boundary = extractBoundary(line);
            debugLog("Extracted boundary: " + boundary);
            if (!boundary.empty()) {
                break;
            }
        }
    }

    if (boundary.empty()) {
        debugLog("Error: Boundary not found in Content-Type header");
        writeStringToSocket(clientSocket, "HTTP/1.1 400 Bad Request\r\n\r\n");
        return;
    }

    debugLog("Processing multipart/form-data with boundary: " + boundary);


    bool inContent = false;
    std::string fieldName;
    std::string filename;
    std::vector<char> fileContent;
    std::string contentDisposition;

    for (const auto &line : requestData) {
        if (inContent) {
            if (startsWith(line, boundary)) {
                debugLog("Found boundary line, ending content section");
                if (!filename.empty()) {
                    debugLog("Saving file: " + filename);
                    std::ofstream outFile(DIRECTORY_PATH + filename, std::ios::binary);
                    if (outFile.is_open()) {
                        outFile.write(fileContent.data(), fileContent.size());
                        outFile.close();
                        debugLog("File saved: " + filename);
                    } else {
                        debugLog("Failed to open file for writing: " + filename);
                    }
                }
                inContent = false;
                fileContent.clear();
                filename.clear();
            } else {
                // Append to file content
                fileContent.insert(fileContent.end(), line.begin(), line.end());
                fileContent.push_back('\n'); // Append newline stripped by request parsing
                debugLog("Appending data to file content buffer");
            }
        } else {
            if (startsWith(line, "Content-Disposition:")) {
                debugLog("Found Content-Disposition header: " + line);
                contentDisposition = line;
                size_t namePos = line.find("name=\"");
                size_t filenamePos = line.find("filename=\"");

                if (namePos != std::string::npos) {
                    namePos += 6; // Length of 'name="'
                    size_t endPos = line.find("\"", namePos);
                    fieldName = line.substr(namePos, endPos - namePos);
                    debugLog("Extracted field name: " + fieldName);
                }

                if (filenamePos != std::string::npos) {
                    filenamePos += 10; // Length of 'filename="'
                    size_t endPos = line.find("\"", filenamePos);
                    filename = line.substr(filenamePos, endPos - filenamePos);
                    debugLog("Extracted file name: " + filename);
                }
            } else if (line == "\r\n") {
                // Start of content
                inContent = !filename.empty();
                debugLog("Starting to read file content for: " + filename);
            }
        }
    }

    writeStringToSocket(clientSocket, "HTTP/1.1 200 OK\r\n\r\n");
    debugLog("POST handling complete");
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

    // set the options
    int _enable = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &_enable, sizeof(_enable)) < 0) {
        /*Do stuff */
    }
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEPORT, &_enable, sizeof(_enable)) < 0) {
        /* Do stuff */
    }

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
            handlePostRequest(clientSocket, requestData);
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
