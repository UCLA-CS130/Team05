#ifndef SERVER_CONFIG_PARSER_H
#define SERVER_CONFIG_PARSER_H

#include "config_parser.h"
#include "http_handler_echo.h"
#include "http_handler_file.h"

struct server_config {
    int port;
    // TODO: Add in more server specific configurations here
};

// The driver that parses a config file for port and request handlers
class NginxServerConfigParser {
public:
    // Constructor: pass in a config file to be parsed for its contents
    NginxServerConfigParser(const char* config_file);
    // Parses all server echo/static file request handlers and stores
    // the parsed handlers in vector out-param. Returns number of request 
    // handlers found
    int parseRequestHandlers(std::vector<http::handler *>* handlers_out);
    // Parses all server settings and stores the parsed server setup
    // in server_config out-param. Returns port number (-1 if no number is
    // found)
    int parseServerSettings(server_config* server_settings_out);
private:
    NginxConfigParser config_parser; // Config parser to parse a file
    NginxConfig config;              // File's config contents after parsing
};

// Returns the port number from config_file or -1 if no number is found
//   TODO: Put this in a class as a static method
int port_number(const char* config_file);

#endif // SERVER_CONFIG_PARSER_H

