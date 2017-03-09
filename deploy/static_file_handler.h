#ifndef STATIC_FILE_HANDLER_H
#define STATIC_FILE_HANDLER_H

#include "request_handler.h"


// Retrieves a file from the server's file system
class StaticFileHandler : public RequestHandler {
public:

    // Initializes the handler. Returns OK if successful
    // uri_prefix is the value in the config file that this handler will run for
    // config is the contents of the child block for this handler ONLY
    virtual Status Init(const std::string& uri_prefix,
                        const NginxConfig& config);

    // Handles an HTTP request, and generates a response. Returns a response code
    // indicating success or failure condition. If ResponseCode is not OK, the
    // contents of the response object are undefined, and the server will return
    // HTTP code 500
    virtual Status HandleRequest(const Request& request, Response* response);

private:

    std::string path_prefix; // Path prefix used by clients to get this handler
    std::string root;        // Root path used for retrieving files
};


REGISTER_REQUEST_HANDLER(StaticFileHandler);

#endif  // STATIC_FILE_HANDLER_H
