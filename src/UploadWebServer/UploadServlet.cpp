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


const std::string DIRECTORY_PATH = "/Users/laurieannesolkoski/CLionProjects/CSAssignment2B/images";
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

// Helper function to determine if a string starts with a given substring
bool startsWith(const std::string &fullString, const std::string &starting) {
    return fullString.compare(0, starting.size(), starting) == 0;
}

// Function to receive and parse HTTP request
std::vector<std::string> receiveAndParseHttpRequest(int clientSocket) {
    debugLog("Receiving HTTP request");
    std::string request;
    char buf;

    // Read the complete request (headers + potential beginning of the body)
    while (true) {
        ssize_t rval = read(clientSocket, &buf, 1);
        if (rval <= 0) {
            debugLog("Failed to receive data or connection closed");
            break;
        }
        request.push_back(buf);
        if (request.size() >= 4 && request.substr(request.size() - 4) == "\r\n\r\n") {
            break; // Headers end detected
        }
    }

    // Split the request into lines for initial processing
    std::vector<std::string> data;
    std::istringstream stream(request);
    std::string line;
    int contentLength = 0;

    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back(); // Remove carriage return character
        }
        data.push_back(line);
        // Extract Content-Length
        if (startsWith(line, "Content-Length:")) {
            std::string lengthStr = line.substr(16); // Get substring after "Content-Length: "
            contentLength = std::stoi(lengthStr); // Convert to integer
        }
    }

    // Read the body of the request based on Content-Length
    std::string body;
    for (int i = 0; i < contentLength; ++i) {
        if (read(clientSocket, &buf, 1) > 0) {
            body.push_back(buf);
        }
    }

    // Add body to data for further processing
    std::istringstream bodyStream(body);
    while (std::getline(bodyStream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        data.push_back(line);
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


// Helper function to extract the boundary from the content type header
std::string extractBoundary(const std::string &contentType) {
    std::string boundaryPrefix = "boundary=";
    size_t boundaryPos = contentType.find(boundaryPrefix);
    if (boundaryPos != std::string::npos) {
        boundaryPos += boundaryPrefix.length();
        return "--" + contentType.substr(boundaryPos); // Ensure only the boundary is extracted
    }
    return "";
}


// Utility function to sanitize and generate a new filename
std::string generateNewFilename(const std::string& caption, const std::string& date, const std::string& originalFilename) {
    auto sanitize = [](const std::string& str) {
        std::string result;
        for (char c : str) {
            if (c == ' ') result += "_";
            else if (isalnum(c) || c == '_' || c == '-') result += c;
        }
        return result;
    };

    std::string sanitizedCaption = sanitize(caption);
    std::string sanitizedDate = sanitize(date);
    std::replace(sanitizedDate.begin(), sanitizedDate.end(), '-', '_');

    std::string extension;
    size_t dotPos = originalFilename.find_last_of(".");
    if (dotPos != std::string::npos) {
        extension = originalFilename.substr(dotPos);
    }

    if (!sanitizedCaption.empty() && !sanitizedDate.empty()) {
        return sanitizedCaption + "_" + sanitizedDate + extension;
    } else if (!sanitizedCaption.empty()) {
        return sanitizedCaption + extension;
    } else {
        return "defaultFilename" + extension; // Replace with a default filename if both caption and date are empty
    }
}



//parse the header
void parseHeaders(const std::vector<std::string>& requestData, std::string& boundary, std::string& fieldName, std::string& fileName) {
    for (const auto &line : requestData) {
        if (startsWith(line, "Content-Type:")) {
            boundary = extractBoundary(line);
        } else if (startsWith(line, "Content-Disposition:")) {
            size_t namePos = line.find("name=\"");
            size_t filenamePos = line.find("filename=\"");

            if (namePos != std::string::npos) {
                namePos += 6; // Length of 'name="' is 6 characters
                size_t endPos = line.find("\"", namePos);
                if (endPos != std::string::npos) {
                    fieldName = line.substr(namePos, endPos - namePos);
                }
                // Optionally, handle the case where endPos is std::string::npos
            }

            if (filenamePos != std::string::npos) {
                filenamePos += 10; // Length of 'filename="' is 10 characters
                size_t endPos = line.find("\"", filenamePos);
                if (endPos != std::string::npos) {
                    fileName = line.substr(filenamePos, endPos - filenamePos);
                }
                // Optionally, handle the case where endPos is std::string::npos
            }
        }
    }
}



//read the binary data
std::vector<char> readBinaryData(int clientSocket, const std::string& boundary) {
    std::vector<char> fileContent;
    std::string readBuffer;
    char buf;
    bool boundaryReached = false;

    while (true) {
        ssize_t rval = read(clientSocket, &buf, 1);
        if (rval <= 0) {
            // Handle errors or closed connection
            break;
        }

        readBuffer.push_back(buf);

        // Check if the read buffer ends with the boundary
        if (readBuffer.size() >= boundary.size() &&
            readBuffer.compare(readBuffer.size() - boundary.size(), boundary.size(), boundary) == 0) {
            boundaryReached = true;
            break;
        }

        // Append to file content
        fileContent.push_back(buf);
    }

    // Remove the boundary from the end of file content
    if (boundaryReached && fileContent.size() >= boundary.size()) {
        fileContent.resize(fileContent.size() - boundary.size());
    }

    return fileContent;
}





//save the file function
void saveFile(const std::string& directoryPath, const std::string& newFileName, const std::vector<char>& fileContent) {
    std::string fullPath = directoryPath + newFileName;
    std::ofstream outFile(fullPath, std::ios::binary);

    if (outFile.is_open()) {
        outFile.write(fileContent.data(), fileContent.size());
        outFile.close();
        debugLog("File saved: " + newFileName);
    } else {
        debugLog("Failed to open file for writing: " + newFileName);
    }
}

//handle the post request
void handlePostRequest(int clientSocket, std::vector<std::string> requestData) {
    debugLog("Entered handlePostRequest");


    std::string boundary, fieldName, fileName, caption, date;
    std::string fieldValue;
    std::vector<char> fileContent;
    bool isReadingFile = false;
    bool pastHeaders = false;

// Extract boundary from the headers
    for (const auto &line : requestData) {
        if (startsWith(line, "Content-Type: multipart/form-data")) {
            boundary = extractBoundary(line);
        }
    }

    // Process each line of request data
    for (const auto &line : requestData) {
        if (startsWith(line, boundary)) {
            pastHeaders = false; // Reset for the next part of the form-data

            if (!fieldName.empty() && isReadingFile && !fileContent.empty()) {
                // End of file content reached
                break;
            } else if (!fieldValue.empty()) {
                if (fieldName == "caption") caption = fieldValue;
                else if (fieldName == "date") date = fieldValue;
                fieldValue.clear(); // Clear the fieldValue for the next field
            }

            isReadingFile = false;
        } else if (startsWith(line, "Content-Disposition:")) {
            parseHeaders({line}, boundary, fieldName, fileName);
            isReadingFile = (fieldName == "file");
        } else if (isReadingFile) {
            if (line == "\r\n" && !pastHeaders) {
                pastHeaders = true; // Start reading the actual content after headers
                continue;
            }
            if (pastHeaders) {
                fileContent.insert(fileContent.end(), line.begin(), line.end());
            }
        } else if (!line.empty() && !isReadingFile) {
            fieldValue += line; // Accumulate field value
        }
    }


    // Generate filename and save file
    if (!fileName.empty()) {
        std::string newFileName = generateNewFilename(caption, date, fileName);
        debugLog("Generated FileName: " + newFileName);
        saveFile(DIRECTORY_PATH, "/" + newFileName, fileContent);
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
        if (mkdir(DIRECTORY_PATH.c_str(), 0700) == -1) {
            debugLog("Failed to create directory: " + DIRECTORY_PATH);
            return 1; // or handle the error appropriately
        }
    }

    startServer();
    debugLog("Server shutdown");
    return 0;
}

