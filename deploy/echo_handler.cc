#include "echo_handler.h"


// Initializes the handler. Returns OK if successful
// uri_prefix is the value in the config file that this handler will run for
// config is the contents of the child block for this handler ONLY
RequestHandler::Status EchoHandler::Init(const std::string& uri_prefix, 
const NginxConfig& config) {
    // Nothing special is needed to initialize the echo handler
    return RequestHandler::OK;
}


// Handles an HTTP request, and generates a response. Returns a response code
// indicating success or failure condition. If ResponseCode is not OK, the
// contents of the response object are undefined, and the server will return
// HTTP code 500
RequestHandler::Status EchoHandler::HandleRequest(const Request& request, 
Response* response) {
    // Respond with the raw request string as plain text
    *response = Response::PlainTextResponse(request.raw_request());
    return RequestHandler::OK;
}

