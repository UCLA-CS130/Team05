#ifndef HTTP_HANDLER_HPP
#define HTTP_HANDLER_HPP


#include <string>

namespace http {

struct response;
struct request;


// Abstract base class for serving all incoming HTTP requests
class handler {
public:

    handler(std::string base_url) { this->base_url = base_url;};

    // Returns a response to the given request
    virtual response handle_request(const request& req) = 0;



    std::string base_url;

};




} // namespace http

#endif // HTTP_HANDLER_HPP

