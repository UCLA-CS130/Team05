#include "http_handler_echo.h"
#include "http_request.h"
#include "http_response.h"


namespace http {

// Echoes the request as plain text
response handler_echo::handle_request(const request& req) {
    // If base url is  not formatted correctly
    if (req.path.length() == 0 || req.path[0] != '/') {
        return response::default_response(response::internal_server_error);
    }

    return http::response::plain_text_response(std::string(req.as_string));
}

} // namespace http

