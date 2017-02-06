#ifndef HTTP_HANDLER_HPP
#define HTTP_HANDLER_HPP


#include <string>

namespace http {

struct response;
struct request;


// Abstract base class for serving all incoming HTTP requests
class handler {
public:

    // Construct handler with the base url such as "/echo" or "/static1"
    handler(const std::string& url) : base_url(url) { };

    // Virtual destructor necessary for deleting derived classes via base class
    virtual ~handler() {};    

    // Returns a response to the given request
    virtual response handle_request(const request& req) = 0;

    std::string base_url; // Base url that corresponds to this handler
};

} // namespace http

#endif // HTTP_HANDLER_HPP

