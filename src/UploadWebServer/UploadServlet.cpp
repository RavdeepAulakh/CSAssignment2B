#include "UploadServlet.hpp"

UploadServlet::UploadServlet() : PORT(8081), DIRECTORY_PATH("./images/") {}

UploadServlet::~UploadServlet() = default;

void UploadServlet::handlePOST(int clientSocket) {
    std::string delimiter = "\r\n\r\n";
    char buffer[4096];
    std::string requestData;

    // Receive POST data from the client
    int bytesRead;
    while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0) {
        requestData.append(buffer, bytesRead);
        if (requestData.find(delimiter) != std::string::npos) {
            break;
        }
    }

    // Parse the POST data to extract form fields and file data
    std::string contentType = "Content-Type: multipart/form-data";
    std::string boundary = requestData.substr(requestData.find(contentType) + contentType.length() + 1);
    boundary = boundary.substr(0, boundary.find("\r\n"));

    // Find the start and end positions of the file data within the request data
    std::size_t start = requestData.find(boundary) + boundary.length() + 2;
    std::size_t end = requestData.find(boundary, start) - 4;

    // Extract file data
    std::string fileData = requestData.substr(start, end - start);

    // Extract form fields (you may need to adjust this logic based on your form structure)
    std::regex regex("name=\"file\"; filename=\"(.+?)\"\r\n\r\n");
    std::smatch match;
    std::string fileName;
    if (std::regex_search(requestData, match, regex)) {
        fileName = match[1];
    } else {
        std::cerr << "Invalid POST request" << std::endl;
        return;
    }

    // Save the uploaded file to the specified directory
    std::ofstream outputFile(DIRECTORY_PATH + fileName, std::ios::out | std::ios::binary);
    if (outputFile.is_open()) {
        outputFile.write(fileData.c_str(), fileData.length());
        outputFile.close();
        std::cout << "File saved: " << DIRECTORY_PATH + fileName << std::endl;
    } else {
        std::cerr << "Failed to save file: " << DIRECTORY_PATH + fileName << std::endl;
    }

    // Send a response back to the client (you may want to send a proper HTTP response)
    std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n";
    send(clientSocket, response.c_str(), response.length(), 0);
}

void UploadServlet::handleGET(int clientSocket) {
    // Prepare and send the HTML form for file upload
    std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
    response += "<html><head><title>File Upload Form</title></head><body>";
    response += "<h1>File Upload Form</h1>";
    response += "<form action='/upload' method='post' enctype='multipart/form-data'>";
    response += "<label for='file'>Select a file:</label>";
    response += "<input type='file' name='file' id='file'><br>";
    response += "<input type='submit' value='Upload'>";
    response += "</form></body></html>";

    send(clientSocket, response.c_str(), response.length(), 0);
}

int main() {
    UploadServlet servlet;

    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t sinSize = sizeof(struct sockaddr_in);

    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address struct
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(servlet.PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket
    if (bind(serverSocket, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) == -1) {
        perror("Binding failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(serverSocket, 10) == -1) {
        perror("Listening failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    std::cout << "Server listening on port " << servlet.PORT << std::endl;

    // Accept incoming connections and handle requests
    while (true) {
        clientSocket = accept(serverSocket, (struct sockaddr*) &clientAddr, &sinSize);
        if (clientSocket == -1) {
            perror("Accepting connection failed");
            close(serverSocket);
            exit(EXIT_FAILURE);
        }

        char request[8192]; // Increased buffer size to handle larger requests
        memset(request, 0, sizeof(request));
        recv(clientSocket, request, sizeof(request), 0);

        std::string requestStr(request);

        // Determine the request method (GET or POST) and handle the request accordingly
        if (requestStr.find("GET") != std::string::npos) {
            servlet.handleGET(clientSocket);
        } else if (requestStr.find("POST") != std::string::npos) {
            servlet.handlePOST(clientSocket);
        }

        close(clientSocket);
    }

    close(serverSocket);
    return 0;
}