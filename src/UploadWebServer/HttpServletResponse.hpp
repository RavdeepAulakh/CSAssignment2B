#ifndef HTTP_SERVLET_RESPONSE_HPP
#define HTTP_SERVLET_RESPONSE_HPP

#include <ostream>
#include <string>

class HttpServletResponse {
private:
    std::ostream& outputStream;
    std::string contentType;
    std::string characterEncoding;

public:
    explicit HttpServletResponse(std::ostream& output) : outputStream(output) {}

    std::ostream& getOutputStream() { return outputStream; }
    void setContentType(const std::string& type) { contentType = type; }
    std::string getContentType() const { return contentType; }
    void setCharacterEncoding(const std::string& encoding) { characterEncoding = encoding; }
    std::string getCharacterEncoding() const { return characterEncoding; }
};

#endif // HTTP_SERVLET_RESPONSE_HPP
