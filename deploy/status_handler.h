#ifndef STATUS_HANDLER_H
#define STATUS_HANDLER_H

#include "request_handler.h"


// Displays information on the status of the web server
class StatusHandler : public RequestHandler {
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
};


REGISTER_REQUEST_HANDLER(StatusHandler);

#endif // STATUS_HANDER_H

