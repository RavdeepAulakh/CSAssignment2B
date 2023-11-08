#ifndef HTTP_SERVLET_REQUEST_HPP
#define HTTP_SERVLET_REQUEST_HPP

#include <istream>

class HttpServletRequest {
private:
    std::istream& inputStream;

public:
    explicit HttpServletRequest(std::istream& input) : inputStream(input) {}
    std::istream& getInputStream() { return inputStream; }
};

#endif // HTTP_SERVLET_REQUEST_HPP
