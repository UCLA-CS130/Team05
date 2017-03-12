// Part of this file is based on the Boost HTTP server example at
//  http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/examples/cpp11_examples.html

#include <fstream>
#include <sstream>
#include <stdio.h>
#include "static_file_handler.h"
#include "not_found_handler.h"
#include "lua.hpp"
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


// Lua function that gets the raw request
static int lua_rawrequest(lua_State* L) {
    lua_pushstring(L, "request");
    lua_rawget(L, LUA_REGISTRYINDEX);
    Request* req = (Request*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    lua_pushstring(L, req->raw_request().c_str());
    return 1;
}


// Lua function that gets the request method
static int lua_method(lua_State* L) {
    lua_pushstring(L, "request");
    lua_rawget(L, LUA_REGISTRYINDEX);
    Request* req = (Request*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    lua_pushstring(L, req->method().c_str());
    return 1;
}


// Lua function that gets the request URI
static int lua_uri(lua_State* L) {
    lua_pushstring(L, "request");
    lua_rawget(L, LUA_REGISTRYINDEX);
    Request* req = (Request*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    lua_pushstring(L, req->uri().c_str());
    return 1;
}


// Lua function that gets the request path
static int lua_path(lua_State* L) {
    lua_pushstring(L, "request");
    lua_rawget(L, LUA_REGISTRYINDEX);
    Request* req = (Request*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    lua_pushstring(L, req->path().c_str());
    return 1;
}


// Lua function that gets the request version
static int lua_version(lua_State* L) {
    lua_pushstring(L, "request");
    lua_rawget(L, LUA_REGISTRYINDEX);
    Request* req = (Request*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    lua_pushstring(L, req->version().c_str());
    return 1;
}


// Lua function that gets the request headers as a Lua table
static int lua_headers(lua_State* L) {
    lua_pushstring(L, "request");
    lua_rawget(L, LUA_REGISTRYINDEX);
    Request* req = (Request*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    using Headers = std::vector<std::pair<std::string, std::string> >;
    Headers headers = req->headers();
    lua_createtable(L, 0, headers.size());
    for (auto it = headers.begin(); it != headers.end(); it++) {
        lua_pushstring(L, it->first.c_str());
        lua_pushstring(L, it->second.c_str());
        lua_rawset(L, -3);
    }
    return 1;
}


// Lua function that gets the request body
static int lua_body(lua_State* L) {
    lua_pushstring(L, "request");
    lua_rawget(L, LUA_REGISTRYINDEX);
    Request* req = (Request*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    lua_pushstring(L, req->body().c_str());
    return 1;
}


// Converts a string containing hex values to a string of just ASCII characters
static std::string hex2ascii(const std::string& in) {
    std::string out;
    out.reserve(in.size());
    for (std::size_t i = 0; i < in.size(); ++i) {
        if (in[i] == '%') {
            if (i + 3 <= in.size()) {
                int value = 0;
                std::istringstream is(in.substr(i + 1, 2));
                if (is >> std::hex >> value) {
                    out += static_cast<char>(value);
                    i += 2;
                } else {
                    out = std::string();
                    return out;
                }
            } else {
                out = std::string();
                return out;
            }
        } else if (in[i] == '+') {
            out += ' ';
        } else {
            out += in[i];
        }
    }
    return out;
}


// Converts form data to a Lua table that is put on the top of the Lua stack
static void form2table(lua_State* L, const std::string& data) {
    // Read the data into a Lua table
    lua_newtable(L);
    std::string key;
    std::string val;
    bool readingkey = true;
    for (char c : data) {
        if (readingkey) {
            if (c == '=') {
                // '=' is the separator between keys and value
                readingkey = false;
            } else {
                key += c;
            }
        } else {
            if (c == '&') {
                // '&' is the separate between key-value pairs
                readingkey = true;

                // Remove hex codes from the key and value then add to Lua table
                lua_pushstring(L, hex2ascii(key).c_str());
                lua_pushstring(L, hex2ascii(val).c_str());
                lua_rawset(L, -3);
                key.clear();
                val.clear();
            } else {
                val += c;
            }
        }
    }

    // Don't forget the last key-value pair
    lua_pushstring(L, hex2ascii(key).c_str());
    lua_pushstring(L, hex2ascii(val).c_str());
    lua_rawset(L, -3);
}


// Lua function that gets form data from the URI of the request
static int lua_get(lua_State* L) {
    lua_pushstring(L, "request");
    lua_rawget(L, LUA_REGISTRYINDEX);
    Request* req = (Request*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    // Read the form data from the URI of the request into a Lua table
    size_t last_question_mark = req->uri().find_last_of("?");
    if (last_question_mark != std::string::npos) {
        form2table(L, req->uri().substr(last_question_mark+1));
    } else {
        lua_newtable(L);
    }
    return 1;
}


// Lua function that gets the post data from the request
static int lua_post(lua_State* L) {
    lua_pushstring(L, "request");
    lua_rawget(L, LUA_REGISTRYINDEX);
    Request* req = (Request*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    // Read the form data from the body of the request into a Lua table
    form2table(L, req->body());
    return 1;
}


// Lua function that adds a header to the response
static int lua_addheader(lua_State* L) {
    lua_pushstring(L, "response");
    lua_rawget(L, LUA_REGISTRYINDEX);
    Response* res = (Response*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    res->AddHeader(luaL_checkstring(L, 1), luaL_checkstring(L, 2));
    return 0;
}


// Lua function that sets the status of the response
static int lua_setstatus(lua_State* L) {
    lua_pushstring(L, "response");
    lua_rawget(L, LUA_REGISTRYINDEX);
    Response* res = (Response*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    int arg = luaL_checkint(L, 1);
    if ((arg >= 200 && arg <= 204) || arg == 301 || arg == 302 || arg == 304 ||
        arg == 400 || arg == 401 || arg == 403 || arg == 404 ||
        (arg >= 500 && arg <= 503)) {
        res->SetStatus((Response::ResponseCode)arg);
    } else {
        luaL_argerror(L, 1, "must be a valid status code");
    }
    return 0;
}


// Lua function that adds the given string to the body of the response
static int lua_put(lua_State* L) {
    lua_pushstring(L, "response_body");
    lua_rawget(L, LUA_REGISTRYINDEX);
    std::string* body = (std::string*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    *body += luaL_checkstring(L, 1);
    return 0;
}


// State used when parsing HTML files for Lua scripts
enum state {
    _start_ignore_immediate_newline,
    _start,
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
    _chunk_whitespace,
    _chunk_whitespace_2,
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
    _expression_whitespace,
    _expression_whitespace_2,
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
static bool do_lua(std::ifstream& file, const Request& request,
Response* response, std::string& body) {
    // Go through the file character by character building the Lua script
    char c;
    std::string lua = "put([[";
    bool next = true;
    state s = _start;
    while (true) {
        // Only move on to the next character if we're supposed to
        if (next) {
            // If we can't read characters anymore, then we're done looping
            if (!file.get(c)) {
                break;
            }
        }

        // By default, we'll keep moving on to the next character
        next = true;

        // Build the Lua script using our state machine
        switch (s) {
        case _start_ignore_immediate_newline: {
            s = _start;
            if (c == '\n') {
                lua += c;
                break;
            }
        }
        case _start: {
            if (c == '<') {
                s = _expecting_question_mark;
            } else if (c == '[' || c == ']') {
                lua += '\\';
                lua += c;
            } else {
                lua += c;
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
                lua += '<';
                next = false;
            }
            break;
        }
        case _expecting_l: {
            if (c == 'l') {
                s = _expecting_u;
            } else {
                s = _start;
                lua += "<?";
                next = false;
            }
            break;
        }
        case _expecting_u: {
            if (c == 'u') {
                s = _expecting_a;
            } else {
                s = _start;
                lua += "<?l";
                next = false;
            }
            break;
        }
        case _expecting_a: {
            if (c == 'a') {
                s = _expecting_space_1;
            } else {
                s = _start;
                lua += "<?lu";
                next = false;
            }
            break;
        }
        case _expecting_space_1: {
            if (c == ' ' || c == '\n') {
                s = _chunk;
                lua += "]])";
                lua += c;
            } else if (c == '=') {
                s = _expecting_space_2;
            } else {
                s = _start;
                lua += "<?lua";
                next = false;
            }
            break;
        }
        case _expecting_space_1_2: {
            if (c == ' ' || c == '\n') {
                s = _chunk_2;
                lua += "]])";
                lua += c;
            } else if (c == '=') {
                s = _expecting_space_2_2;
            } else {
                s = _start;
                lua += "<%";
                next = false;
            }
            break;
        }
        case _expecting_space_2: {
            if (c == ' ' || c == '\n') {
                s = _expression;
                lua += "]])";
                lua += c;
                lua += "put(";
            } else {
                s = _start;
                lua += "<lua=";
                next = false;
            }
            break;
        }
        case _expecting_space_2_2: {
            if (c == ' ' || c == '\n') {
                s = _expression_2;
                lua += "]])";
                lua += c;
                lua += "put(";
            } else {
                s = _start;
                lua += "<%=";
                next = false;
            }
            break;
        }
        case _chunk: {
            if (c == '\'') {
                s = _chunk_single_quote;
            } else if (c == '"') {
                s = _chunk_double_quote;
            } else if (c == ' ' || c == '\n') {
                s = _chunk_whitespace;
            }
            lua += c;
            break;
        }
        case _chunk_2: {
            if (c == '\'') {
                s = _chunk_single_quote_2;
            } else if (c == '"') {
                s = _chunk_double_quote_2;
            } else if (c == ' ' || c == '\n') {
                s = _chunk_whitespace_2;
            }
            lua += c;
            break;
        }
        case _chunk_whitespace: {
            if (c == '?') {
                s = _chunk_question_mark;
            } else if (c == ' ' || c == '\n') {
                lua += c;
            } else {
                s = _chunk;
                next = false;
            }
            break;
        }
        case _chunk_whitespace_2: {
            if (c == '%') {
                s = _chunk_question_mark_2;
            } else if (c == ' ' || c == '\n') {
                lua += c;
            } else {
                s = _chunk_2;
                next = false;
            }
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
                lua += "put([[";
            } else {
                s = _chunk;
                lua += '?';
                next = false;
            }
            break;
        }
        case _chunk_question_mark_2: {
            if (c == '>') {
                s = _start_ignore_immediate_newline;
                lua += "put([[";
            } else {
                s = _chunk_2;
                lua += '%';
                next = false;
            }
            break;
        }
        case _expression: {
            if (c == '\'') {
                s = _expression_single_quote;
            } else if (c == '"') {
                s = _expression_double_quote;
            } else if (c == ' ' || c == '\n') {
                s = _expression_whitespace;
            }
            lua += c;
            break;
        }
        case _expression_2: {
            if (c == '\'') {
                s = _expression_single_quote_2;
            } else if (c == '"') {
                s = _expression_double_quote_2;
            } else if (c == ' ' || c == '\n') {
                s = _expression_whitespace_2;
            }
            lua += c;
            break;
        }
        case _expression_whitespace: {
            if (c == '?') {
                s = _expression_question_mark;
            } else if (c == ' ' || c == '\n') {
                lua += c;
            } else {
                s = _expression;
                next = false;
            }
            break;
        }
        case _expression_whitespace_2: {
            if (c == '%') {
                s = _expression_question_mark_2;
            } else if (c == ' ' || c == '\n') {
                lua += c;
            } else {
                s = _expression_2;
                next = false;
            }
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
                lua.insert(lua.end() - 1, ')');
                lua += "put([[";
            } else {
                s = _expression;
                lua += '?';
                next = false;
            }
            break;
        }
        case _expression_question_mark_2: {
            if (c == '>') {
                s = _start_ignore_immediate_newline;
                lua.insert(lua.end() - 1, ')');
                lua += "put([[";
            } else {
                s = _expression_2;
                lua += '%';
                next = false;
            }
            break;
        }
        default:
            break;
        }
    }
    lua += "]])";

    // Open a new Lua virtual machine
    lua_State* L = lua_open();
    luaL_openlibs(L);

    // Add our request reading functions to the Lua VM
    lua_pushstring(L, "request");
    lua_pushlightuserdata(L, const_cast<Request*>(&request));
    lua_rawset(L, LUA_REGISTRYINDEX);
    lua_register(L, "rawrequest", lua_rawrequest);
    lua_register(L, "method", lua_method);
    lua_register(L, "uri", lua_uri);
    lua_register(L, "path", lua_path);
    lua_register(L, "version", lua_version);
    lua_register(L, "headers", lua_headers);
    lua_register(L, "body", lua_body);
    lua_register(L, "get", lua_get);
    lua_register(L, "post", lua_post);

    // Add our response writing functions to the Lua VM
    lua_pushstring(L, "response");
    lua_pushlightuserdata(L, response);
    lua_rawset(L, LUA_REGISTRYINDEX);
    lua_register(L, "addheader", lua_addheader);
    lua_register(L, "setstatus", lua_setstatus);

    // Add our put function to the Lua VM
    lua_pushstring(L, "response_body");
    lua_pushlightuserdata(L, &body);
    lua_rawset(L, LUA_REGISTRYINDEX);
    lua_register(L, "put", lua_put);

    // Run the Lua code, which will build the body of the response
    //printf("%s\n", lua.c_str());
    if ((luaL_loadstring(L, lua.c_str()) || lua_pcall(L, 0, 0, 0)) != 0) {
        // A Lua error has occurred
        printf("%s\n", lua_tostring(L, -1));
        lua_close(L);
        return false;
    }

    // Close out the Lua VM
    lua_close(L);

    return true;
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
    // Remove the path prefix from the path to get the file path + post data
    std::string path = request.path();
    path = path.substr(path_prefix.size());

    // Remove the post data from the path to get the actaul file path
    std::size_t first_question_mark_pos = path.find("?");
    if (first_question_mark_pos != std::string::npos) {
        path = path.substr(0, first_question_mark_pos);
    }

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
        printf("Could not find file %s\n", full_path.c_str());
        NotFoundHandler h;
        return h.HandleRequest(request, response);
    }

    // Check if were responding with an HTML file
    std::string body;
    if (type == "text/html") {
        // True : process any Lua scripts within the file before responding
        response->SetStatus(Response::ok);
        if (!do_lua(file, request, response, body)) {
            return RequestHandler::Error;
        } else if (file.bad()) {
            printf("Error ocurred during reading file\n");
            return RequestHandler::Error;
        }

        // Convert markdown to html if extension is .md
        if (extension == "md") {
            // Read in markdown body to convert to html
            markdown::Document doc;
            doc.read(body);

            // Markdown write function outputs to an ostream so       
            // convert to string
            std::ostringstream stream;
            doc.write(stream);
            std::string md_html = stream.str();
            
            body = md_html;
        }
    } else {
        // False : just add the file as is without any extra processing
        response->SetStatus(Response::ok);
        char buf[512];
        while (file.read(buf, sizeof(buf)).gcount() > 0) {
            body.append(buf, file.gcount());
        }
        if (file.bad()) {
            printf("Error ocurred during reading file\n");
            return RequestHandler::Error;
        }
    }
    file.close();

    // Fill out the rest of the response
    response->SetBody(body);
    response->AddHeader("Content-Length",
        std::to_string(response->GetBody().size()));
    response->AddHeader("Content-Type", type);
    return RequestHandler::OK;
}

