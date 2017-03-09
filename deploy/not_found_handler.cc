#include "not_found_handler.h"


// Initializes the handler. Returns OK if successful
// uri_prefix is the value in the config file that this handler will run for
// config is the contents of the child block for this handler ONLY
RequestHandler::Status NotFoundHandler::Init(const std::string& uri_prefix, 
const NginxConfig& config) {
    // Nothing special is needed to initialize the 404 handler
    //  Note: StaticFileHandler does not call Init when it uses NotFoundHandler
    return RequestHandler::OK;
}


// Handles an HTTP request, and generates a response. Returns a response code
// indicating success or failure condition. If ResponseCode is not OK, the
// contents of the response object are undefined, and the server will return
// HTTP code 500
RequestHandler::Status NotFoundHandler::HandleRequest(const Request& request, 
Response* response) {
    // Respond with the default 404 not found response
    *response = Response::DefaultResponse(Response::not_found);
    return RequestHandler::OK;
}

