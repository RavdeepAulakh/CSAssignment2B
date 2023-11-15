#include "HttpServletResponse.h"
#include <unistd.h> 

using namespace std;

// Set the HTTP status line with the status code and reason phrase
void HttpServletResponse::setStatus(int statusCode, const std::string &reason) {
    status = "HTTP/1.1 " + to_string(statusCode) + " " + reason + "\r\n";
}

// Add a header to the internal map of headers
void HttpServletResponse::addHeader(const std::string &name, const std::string &value) {
    headers[name] = value; // Use the name as the key and the value as the value in the map
}

// Write data to the response body buffer
void HttpServletResponse::writeResponse(const char *buffer, size_t bufferSize) {
    // Insert the data into the body buffer at the end
    body.insert(body.end(), buffer, buffer + bufferSize);
}

// Send the complete HTTP response
void HttpServletResponse::sendResponse() {
    // Send the status line first
    write(sockfd, status.c_str(), status.size());

    // Send the headers
    for (const auto &header : headers) {
        string headerLine = header.first + ": " + header.second + "\r\n";
        write(sockfd, headerLine.c_str(), headerLine.size());
    }

    // End of headers, send a blank line
    write(sockfd, "\r\n", 2);

    // Send the body if it's not empty
    if (!body.empty()) {
        write(sockfd, &body[0], body.size());
    }
}


/*
void HttpServletResponse::sendResponse() {
    std::ostringstream responseStream;

    // Construct the response
    responseStream << status;
    for (const auto &header : headers) {
        responseStream << header.first << ": " << header.second << "\r\n";
    }
    // Add Content-Length if there's a body
    if (!body.empty()) {
        responseStream << "Content-Length: " << body.size() << "\r\n";
    }
    responseStream << "\r\n"; // End of headers
    if (!body.empty()) {
        responseStream.write(&body[0], body.size());
    }

    // Convert the stream to a string and send it
    std::string response = responseStream.str();
    ssize_t bytesWritten = write(sockfd, response.c_str(), response.size());
    if (bytesWritten < 0) {
        // Handle error, e.g., logging or throwing an exception
    }
}

*/