#ifndef HTTP_HANDLER_FILE_HPP
#define HTTP_HANDLER_FILE_HPP

#include "http_handler.h"


namespace http {

struct response;
struct request;


// Handler for serving static files
class handler_file : public handler {
public:

    // Constructor used for choosing the root path for the handler
    handler_file(const std::string& doc_root, const std::string& base_url);

    // Responds with the file requested or an error page
    response handle_request(const request& req);

private:

    std::string root; // Root path for the file handler
};

} // namespace http

#endif // HTTP_HANDLER_FILE_HPP

