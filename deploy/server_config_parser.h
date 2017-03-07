#ifndef SERVER_CONFIG_PARSER_H
#define SERVER_CONFIG_PARSER_H

#include "config_parser.h"
#include "request_handler.h"

struct ServerSettings {
    int port;
    // Add in more server specific configurations here
};

// The driver that parses a config file for port and request handlers
class NginxServerConfigParser {
public:

    // Constructor: pass in a config file to be parsed for its contents
    NginxServerConfigParser(const char* config_file);

    // Parses all server request handlers and stores the parsed handlers in 
    // map out-param. Returns number of request handlers found
    int ParseRequestHandlers(
        std::map<std::string, std::unique_ptr<RequestHandler> >& handlers_out,
        std::map<std::string, std::string>& handler_info_out);

    // Parses all server settings and stores the parsed server setup
    // in server_config out-param. Returns port number (-1 if no number is
    // found)
    int ParseServerSettings(ServerSettings& server_settings_out);

private:

    NginxConfig       config;        // File's config contents after parsing
    NginxConfigParser config_parser; // Config parser to parse a file
};

#endif // SERVER_CONFIG_PARSER_H

