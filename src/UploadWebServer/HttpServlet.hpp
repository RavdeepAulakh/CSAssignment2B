#ifndef HTTP_SERVLET_HPP
#define HTTP_SERVLET_HPP

#include "HttpServletRequest.hpp"
#include "HttpServletResponse.hpp"

class HttpServlet {
public:
    virtual ~HttpServlet() {}

    // Purely virtual functions
    virtual void doGet(HttpServletRequest& request, HttpServletResponse& response) = 0;
    virtual void doPost(HttpServletRequest& request, HttpServletResponse& response) = 0;
};

#endif // HTTP_SERVLET_HPP
