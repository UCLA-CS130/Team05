// This file is based on the Boost HTTP server example at
//  http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/examples/cpp11_examples.html

#include <fstream>
#include <stdio.h>
#include "static_file_handler.h"
#include "not_found_handler.h"
#include "cpp-markdown/markdown.h"

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
    { "png", "image/png" },
    { "md", "text/html" }
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


// Initializes the handler. Returns OK if successful
// uri_prefix is the value in the config file that this handler will run for
// config is the contents of the child block for this handler ONLY
RequestHandler::Status StaticFileHandler::Init(const std::string& uri_prefix, 
const NginxConfig& config) {
    // Check config child block for settings
    for (size_t i = 0; i < config.statements.size(); i++) {
        // Only attempt to read statements with a first token we care about
        if (config.statements[i]->tokens.size() > 0 &&
            config.statements[i]->tokens[0] == "root") {
            // Root must have 1 extra token past the first "root" one
            if (config.statements[i]->tokens.size() > 1) {
                root = config.statements[i]->tokens[1];
            } else {
                printf("\"root\" statement in config needs more tokens\n");
            }
        }
    }
    if (!root.empty()) {
        // Store the path prefix for later use
        path_prefix = uri_prefix;
        return RequestHandler::OK;
    } else {
        printf("No root provided to StaticFileHandler\n");
        return RequestHandler::Error;
    }

    
}


// Handles an HTTP request, and generates a response. Returns a response code
// indicating success or failure condition. If ResponseCode is not OK, the
// contents of the response object are undefined, and the server will return
// HTTP code 500
RequestHandler::Status StaticFileHandler::HandleRequest(const Request& request, 
Response* response) {
    // Remove the path prefix from the path to get the file path
    std::string path = request.path();
    path = path.substr(path_prefix.size());

    printf("path_prefix:%s, original path:%s, new path:%s\n", path_prefix.c_str(), request.path().c_str(), path.c_str() );
    // Determine the file extension
    std::size_t last_slash_pos = path.find_last_of("/");
    std::size_t last_dot_pos   = path.find_last_of(".");
    std::string extension;
    if (last_dot_pos != std::string::npos &&
        (last_slash_pos == std::string::npos || 
         last_dot_pos > last_slash_pos)) {
        extension = path.substr(last_dot_pos + 1);
    }
    std::string type = extension_to_type(extension);

    // Open the file to send back
    std::string full_path = root + path;
    printf("File path: %s|\n", full_path.c_str());
    printf("Extension Type: %s|\n", extension.c_str());
    printf("Content Type: %s|\n\n", type.c_str());
    std::ifstream file(full_path.c_str(), std::ios::in | std::ios::binary);
    if (!file) {
        // Or, use the NotFoundHandler
        NotFoundHandler h;
        return h.HandleRequest(request, response);
    }

    // Fill out the response to be sent to the client
    std::string body;
    char buf[512];
    while (file.read(buf, sizeof(buf)).gcount() > 0) {
        body.append(buf, file.gcount());
    }
    // Convert markdown to html file if extension is .md
    // Otherwise, set the body normally
    if (extension == "md") {
        // Read in markdown body to convert to html
        markdown::Document doc;
        doc.read(body);

        // Markdown write function outputs to an ostream so convert to string
        std::ostringstream stream;
        doc.write(stream);
        std::string md_html = stream.str();

        response->SetBody(md_html);
    } else {
        response->SetBody(body);
    }
    response->SetStatus(Response::ok);
    response->AddHeader("Content-Length", std::to_string(response->GetBody().size()));
    response->AddHeader("Content-Type", type);
    return RequestHandler::OK;
}

