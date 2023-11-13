#ifndef HTTP_SERVLET_RESPONSE_HPP
#define HTTP_SERVLET_RESPONSE_HPP

#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

class HttpServletResponse {
private:
    int sockfd; // Socket file descriptor for sending data
    unordered_map<string, string> headers; // Stores all HTTP headers
    vector<char> body; // Char buffer for the response body
    string status; // String for HTTP status 

public:
    HttpServletResponse(int s) : sockfd(s) {}

    // Sets the HTTP status line
    void setStatus(int statusCode, const string &reason);

    // Adds an HTTP header to the response
    void addHeader(const string &name, const string &value);

    void writeResponse(const char* buffer, size_t bufferSize);

    void sendResponse();

    ~HttpServletResponse() {}
};

#endif // HTTP_SERVLET_RESPONSE_HPP
