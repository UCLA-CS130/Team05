// This file is based on the Boost HTTP server example at
//  http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/examples/cpp11_examples.html

#include <fstream>
#include "http_handler_file.h"
#include "http_request.h"
#include "response.h"


namespace http {


// Structure for mapping extensions to content types
struct mapping {
    const char* extension;
    const char* content_type;
};


// The actual mappings defined for the webserver
static mapping mappings[] = {
    { "gif", "image/gif" },
    { "htm", "text/html" },
    { "html", "text/html" },
    { "jpg", "image/jpeg" },
    { "png", "image/png" }
};


// Convert a file extension into a content type
static std::string extension_to_type(const std::string& extension) {
    for (mapping m: mappings) {
        if (m.extension == extension) {
            return m.content_type;
        }
    }
    return "text/plain";
}


// Constructor used for choosing the root path for the handler
handler_file::handler_file(const std::string& doc_root, const std::string& base_url) : handler(base_url), root(doc_root) {

}


// Responds with the file requested or an error page
Response handler_file::handle_request(const request& req) {
    // If base url is  not formatted correctly
    if (req.path.length() == 0 || req.path[0] != '/') {
        return Response::default_response(Response::internal_server_error);
    }

    // Determine the file extension
    std::size_t last_slash_pos = req.path.find_last_of("/");
    std::size_t last_dot_pos = req.path.find_last_of(".");
    std::string extension;
    if (last_dot_pos != std::string::npos && last_dot_pos > last_slash_pos) {
        extension = req.path.substr(last_dot_pos + 1);
    }

    // The request's base url, skipping first /
    std::string reqs_base_url = req.path.substr(0, req.path.find("/", 1));

    // If the base url's do not match, this handler should not have been called
    if (reqs_base_url != this->base_url) {
        return Response::default_response(Response::internal_server_error);
    }

    std::string file_path = req.path.substr(this->base_url.length());

    // For if base url is provided by user but nothing else
    if (file_path == "") {
        return Response::default_response(Response::not_found);
    }

    // Open the file to send back
    std::string full_path = root + file_path;
    printf("|File path: %s|\n", full_path.c_str());
    printf("|Extension Type: %s|\n", extension.c_str());
    printf("|Extension Type: %s|\n", extension_to_type(extension).c_str());

    std::ifstream file(full_path.c_str(), std::ios::in | std::ios::binary);
    if (!file) {
        return Response::default_response(Response::not_found);
    }

    // Fill out the response to be sent to the client
    Response res;
    res.SetStatus(Response::ok);
    std::string res_body;
    // res.status = Response::ok;
    char buf[512];
    while (file.read(buf, sizeof(buf)).gcount() > 0) {
        res_body.append(buf, file.gcount());
    }
    res.SetBody(res_body);
    // res.headers.resize(2);
    res.AddHeader("Content-Length", std::to_string(res.GetBody().size()));
    res.AddHeader("Content-Type", extension_to_type(extension));
    // res.headers[0].name = "Content-Length";
    // res.headers[0].value = std::to_string(res.content.size());
    // res.headers[1].name = "Content-Type";
    // res.headers[1].value = extension_to_type(extension);

    return res;
}

} // namespace http
