// This file is based on the Boost HTTP server example at
//  http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/examples/cpp11_examples.html

#include <fstream>
#include <stdio.h>
#include "static_file_handler.h"
#include "not_found_handler.h"
#include "lua.hpp"


// State used when parsing HTML files for Lua scripts
enum state {
    _start,
    _start_ignore_immediate_newline,
    _expecting_question_mark,
    _expecting_l,
    _expecting_u,
    _expecting_a,
    _expecting_space_1,
    _expecting_space_1_2,
    _expecting_space_2,
    _expecting_space_2_2,
    _chunk,
    _chunk_2,
    _chunk_single_quote,
    _chunk_single_quote_2,
    _chunk_single_quote_escape,
    _chunk_single_quote_escape_2,
    _chunk_double_quote,
    _chunk_double_quote_2,
    _chunk_double_quote_escape,
    _chunk_double_quote_escape_2,
    _chunk_question_mark,
    _chunk_question_mark_2,
    _expression,
    _expression_2,
    _expression_single_quote,
    _expression_single_quote_2,
    _expression_single_quote_escape,
    _expression_single_quote_escape_2,
    _expression_double_quote,
    _expression_double_quote_2,
    _expression_double_quote_escape,
    _expression_double_quote_escape_2,
    _expression_question_mark,
    _expression_question_mark_2
};


// Read through file character-by-character processing Lua scripts into body
static void do_lua(std::ifstream& file, std::string& body) {
    // Open an instance of the Lua virtual machine
    lua_State* L = lua_open();
    luaL_openlibs(L);

    // Go through the file character by character
    char c;
    std::string lua;
    state s = _start;
    while (file.get(c)) {
        // Look for different characters based on what state were in
        switch (s) {
        case _start: {
            if (c == '<') {
                s = _expecting_question_mark;
            } else {
                body += c;
            }
            break;
        }
        case _start_ignore_immediate_newline: {
            if (c == '<') {
                s = _expecting_question_mark;
            } else if (c == '\n') {
                s = _start;
            } else {
                s = _start;
                body += c;
            }
            break;
        }
        case _expecting_question_mark: {
            if (c == '?') {
                s = _expecting_l;
            } else if (c == '%') {
                s = _expecting_space_1_2;
            } else {
                s = _start;
                body += '<';
                body += c;
            }
            break;
        }
        case _expecting_l: {
            if (c == 'l') {
                s = _expecting_u;
            } else {
                s = _start;
                body += "<?";
                body += c;
            }
            break;
        }
        case _expecting_u: {
            if (c == 'u') {
                s = _expecting_a;
            } else {
                s = _start;
                body += "<?l";
                body += c;
            }
            break;
        }
        case _expecting_a: {
            if (c == 'a') {
                s = _expecting_space_1;
            } else {
                s = _start;
                body += "<?lu";
                body += c;
            }
            break;
        }
        case _expecting_space_1: {
            if (c == ' ' || c == '\n') {
                s = _chunk;
            } else if (c == '=') {
                s = _expecting_space_2;
            } else {
                s = _start;
                body += "<?lua";
                body += c;
            }
            break;
        }
        case _expecting_space_1_2: {
            if (c == ' ' || c == '\n') {
                s = _chunk_2;
            } else if (c == '=') {
                s = _expecting_space_2_2;
            } else {
                s = _start;
                body += "<%";
                body += c;
            }
            break;
        }
        case _expecting_space_2: {
            if (c == ' ' || c == '\n') {
                s = _expression;
                lua += "return ";
            } else {
                s = _start;
                body += "<lua=";
                body += c;
            }
            break;
        }
        case _expecting_space_2_2: {
            if (c == ' ' || c == '\n') {
                s = _expression_2;
                lua += "return ";
            } else {
                s = _start;
                body += "<%=";
                body += c;
            }
            break;
        }
        case _chunk: {
            if (c == '\'') {
                s = _chunk_single_quote;
            } else if (c == '"') {
                s = _chunk_double_quote;
            } else if (c == '?') {
                s = _chunk_question_mark;
                break;
            }
            lua += c;
            break;
        }
        case _chunk_2: {
            if (c == '\'') {
                s = _chunk_single_quote_2;
            } else if (c == '"') {
                s = _chunk_double_quote_2;
            } else if (c == '%') {
                s = _chunk_question_mark_2;
                break;
            }
            lua += c;
            break;
        }
        case _chunk_single_quote: {
            if (c == '\'') {
                s = _chunk;
            } else if (c == '\\') {
                s = _chunk_single_quote_escape;
            }
            lua += c;
            break;
        }
        case _chunk_single_quote_2: {
            if (c == '\'') {
                s = _chunk_2;
            } else if (c == '\\') {
                s = _chunk_single_quote_escape_2;
            }
            lua += c;
            break;
        }
        case _chunk_single_quote_escape: {
            s = _chunk_single_quote;
            lua += c;
            break;
        }
        case _chunk_single_quote_escape_2: {
            s = _chunk_single_quote_2;
            lua += c;
            break;
        }
        case _chunk_double_quote: {
            if (c == '"') {
                s = _chunk;
            } else if (c == '\\') {
                s = _chunk_double_quote_escape;
            }
            lua += c;
            break;
        }
        case _chunk_double_quote_2: {
            if (c == '"') {
                s = _chunk_2;
            } else if (c == '\\') {
                s = _chunk_double_quote_escape_2;
            }
            lua += c;
            break;
        }
        case _chunk_double_quote_escape: {
            s = _chunk_double_quote;
            lua += c;
            break;
        }
        case _chunk_double_quote_escape_2: {
            s = _chunk_double_quote_2;
            lua += c;
            break;
        }
        case _chunk_question_mark: {
            if (c == '>') {
                s = _start_ignore_immediate_newline;

                // Run the Lua chunk and prepare to receive another one
                if ((luaL_loadstring(L, lua.c_str()) ||
                     lua_pcall(L, 0, 0, 0)) != 0) {
                    // A Lua error has occurred
                    printf("%s\n", lua_tostring(L, -1));
                    lua_pop(L, 1); // Prevent memory leak
                }
                lua.clear();
            } else {
                s = _chunk;
                lua += '?';
                lua += c;
            }
            break;
        }
        case _chunk_question_mark_2: {
            if (c == '>') {
                s = _start_ignore_immediate_newline;

                // Run the Lua chunk and prepare to receive another one
                if ((luaL_loadstring(L, lua.c_str()) ||
                     lua_pcall(L, 0, 0, 0)) != 0) {
                    // A Lua error has occurred
                    printf("%s\n", lua_tostring(L, -1));
                    lua_pop(L, 1); // Prevent memory leak
                }
                lua.clear();
            } else {
                s = _chunk_2;
                lua += '%';
                lua += c;
            }
            break;
        }
        case _expression: {
            if (c == '\'') {
                s = _expression_single_quote;
            } else if (c == '"') {
                s = _expression_double_quote;
            } else if (c == '?') {
                s = _expression_question_mark;
                break;
            }
            lua += c;
            break;
        }
        case _expression_2: {
            if (c == '\'') {
                s = _expression_single_quote_2;
            } else if (c == '"') {
                s = _expression_double_quote_2;
            } else if (c == '%') {
                s = _expression_question_mark_2;
                break;
            }
            lua += c;
            break;
        }
        case _expression_single_quote: {
            if (c == '\'') {
                s = _expression;
            } else if (c == '\\') {
                s = _expression_single_quote_escape;
            }
            lua += c;
            break;
        }
        case _expression_single_quote_2: {
            if (c == '\'') {
                s = _expression_2;
            } else if (c == '\\') {
                s = _expression_single_quote_escape_2;
            }
            lua += c;
            break;
        }
        case _expression_single_quote_escape: {
            s = _expression_single_quote;
            lua += c;
            break;
        }
        case _expression_single_quote_escape_2: {
            s = _expression_single_quote_2;
            lua += c;
            break;
        }
        case _expression_double_quote: {
            if (c == '"') {
                s = _expression;
            } else if (c == '\\') {
                s = _expression_double_quote_escape;
            }
            lua += c;
            break;
        }
        case _expression_double_quote_2: {
            if (c == '"') {
                s = _expression_2;
            } else if (c == '\\') {
                s = _expression_double_quote_escape_2;
            }
            lua += c;
            break;
        }
        case _expression_double_quote_escape: {
            s = _expression_double_quote;
            lua += c;
            break;
        }
        case _expression_double_quote_escape_2: {
            s = _expression_double_quote_2;
            lua += c;
            break;
        }
        case _expression_question_mark: {
            if (c == '>') {
                s = _start_ignore_immediate_newline;

                // Add the expression result to the HTML body
                if ((luaL_loadstring(L, lua.c_str()) ||
                     lua_pcall(L, 0, 1, 0)) != 0) {
                    // A Lua error has occurred
                    printf("%s\n", lua_tostring(L, -1));
                } else if (lua_isstring(L, -1)) {
                    body += lua_tostring(L, -1);
                }
                lua_pop(L, 1); // Prevent memory leak
                lua.clear();
            } else {
                s = _expression;
                lua += '?';
                lua += c;
            }
            break;
        }
        case _expression_question_mark_2: {
            if (c == '>') {
                s = _start_ignore_immediate_newline;

                // Add the expression result to the HTML body
                if ((luaL_loadstring(L, lua.c_str()) ||
                     lua_pcall(L, 0, 1, 0)) != 0) {
                    // A Lua error has occurred
                    printf("%s\n", lua_tostring(L, -1));
                } else if (lua_isstring(L, -1)) {
                    body += lua_tostring(L, -1);
                }
                lua_pop(L, 1); // Prevent memory leak
                lua.clear();
            } else {
                s = _expression_2;
                lua += '%';
                lua += c;
            }
            break;
        }
        default:
            break;
        }
    }
}


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
    std::ifstream file(full_path.c_str(), std::ios::in | std::ios::binary);
    if (!file) {
        // Or, use the NotFoundHandler
        NotFoundHandler h;
        return h.HandleRequest(request, response);
    }

    // Check if were responding with an HTML file
    std::string body;
    if (type == "text/html") {
        // True : process any Lua scripts within the file before responding
        do_lua(file, body);
        if (file.bad()) {
            printf("Error ocurred during reading file\n");
        }
    } else {
        // False : just add the file as is without any extra processing
        char buf[512];
        while (file.read(buf, sizeof(buf)).gcount() > 0) {
            body.append(buf, file.gcount());
        }
        if (file.bad()) {
            printf("Error ocurred during reading file\n");
        }
    }
    file.close();

    // Fill out the rest of the response
    response->SetBody(body);
    response->AddHeader("Content-Length",
        std::to_string(response->GetBody().size()));
    response->SetStatus(Response::ok);
    response->AddHeader("Content-Type", type);
    return RequestHandler::OK;
}

