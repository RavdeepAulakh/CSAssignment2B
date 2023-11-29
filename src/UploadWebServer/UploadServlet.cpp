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
#include <iostream>


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

//save the file function
void saveFile(const std::string& directoryPath, const std::string& fileName, const std::vector<char>& fileContent) {
    std::string fullPath = directoryPath + "/" + fileName;
    std::ofstream outFile(fullPath, std::ios::binary);

    if (outFile.is_open()) {
        outFile.write(fileContent.data(), fileContent.size());
        outFile.close();
        debugLog("File saved: " + fullPath);
    } else {
        debugLog("Failed to open file for writing: " + fullPath);
    }
}



// Function to generate the response HTML
std::string getResponse(const std::string& directoryPath) {
    // Generate HTML list of files in the directory
    std::string filesListHTML = listFilesAsHTML(directoryPath);

    // Create the HTML response
    std::ostringstream html;
    html << "HTTP/1.1 200 OK\r\n"
         << "Content-Type: text/html\r\n"
         << "\r\n"
         << "<html><head><title>File Upload Result</title></head><body>"
         << filesListHTML
         << "</body></html>";

    return html.str();
}

//handle the post request
void handlePostRequest(int clientSocket, const std::vector<std::string>& requestData) {
    debugLog("Handling POST request");

    std::string boundary, fileName, currentField, caption, date, fieldValue;
    bool isFileContent = false;
    bool isReadingValue = false;
    bool isFirstLine = true;
    bool skipLine = false; // Flag to skip the Content-Type line
    std::vector<char> fileContent;


    for (const auto& line : requestData) {
        debugLog("Request Line: " + line);
        // Extract boundary
        if (startsWith(line, "Content-Type: multipart/form-data")) {
            boundary = extractBoundary(line);
            debugLog("Boundary: " + boundary);
        }


        // Find filename, if the string "line" contains the filename field, grab the filename
        if (startsWith(line, "Content-Disposition: form-data;") && line.find("filename=\"") != std::string::npos) {
            debugLog("FIELDLINE HERE: filename!!!");
            size_t startPos = line.find("filename=\"") + 10;
            size_t endPos = line.find("\"", startPos);
            fileName = line.substr(startPos, endPos - startPos);
            isFileContent = true; // The next non-empty line after this is file content
            skipLine = true; // Skip the next line (Content-Type)
            debugLog("Found file name: " + fileName);
            continue;
        }

        // Check for caption or date field
        if (startsWith(line, "Content-Disposition: form-data;")) {
            debugLog("FOUND FORMDATA");
            if (line.find("name=\"caption\"") != std::string::npos) {
                currentField = "caption";
            } else if (line.find("name=\"date\"") != std::string::npos) {
                currentField = "date";
            }
            continue;
        }

        // Capture the value for caption or date
        if (!currentField.empty() && !line.empty() && line != "\r\n") {
            if (currentField == "caption") {
                caption = line;
                debugLog("CAPTION = " + caption);
                currentField.clear();
            } else if (currentField == "date") {
                date = line;
                debugLog("DATE = " + date);
                currentField.clear();
            }
        }

        if (isFileContent) {
            if (line == boundary || line == (boundary + "--")) {
                if (!fileContent.empty()) {
                    fileContent.pop_back();
                }
                isFileContent = false; // Reset isFileContent for other form fields
                continue;
            }
            if (skipLine) {
                skipLine = false; // Skip this line and reset the flag
                continue;
            }
            if (isFirstLine) {
                isFirstLine = false;
                continue;
            }
            // Insert the line content directly, including empty lines
            fileContent.insert(fileContent.end(), line.begin(), line.end());
            // Add a newline character for each line, regardless of its content
            fileContent.push_back('\n');
        }
    }

    debugLog("File content size: " + std::to_string(fileContent.size()));

    // Save the file
    if (!fileContent.empty() && !fileName.empty()) {
        std::string newFileName = generateNewFilename(caption, date, fileName);
        saveFile(DIRECTORY_PATH, newFileName, fileContent);
    } else {
        debugLog("No file content found or file name is empty.");
    }

    // Generate and send response to client
    std::string response = getResponse(DIRECTORY_PATH);
    writeStringToSocket(clientSocket, response);
    debugLog("POST request handling complete");
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



