#include "server_config_parser.h"
#include "http_handler_echo.h"
#include "http_handler_file.h"

// Constructor: pass in a config file to be parsed for its contents
NginxServerConfigParser::NginxServerConfigParser(const char* config_file) {
    // Parses the config file into statements and tokens for us to extract data
    config_parser.Parse(config_file, &config);
}

// Parses all server echo/static file request handlers and stores the parsed
// handlers in a vector out-param. Returns number of request handlers found
int NginxServerConfigParser::parseRequestHandlers(
    std::vector<http::handler *>* handlers_out) {
    
    // Check config file for echo/static file requests to add
    for (size_t i = 0; i < config.statements.size(); i++) {
        // Add echo handler with its base URL
        if (config.statements[i]->tokens[0] == "echo") {
            http::handler_echo handler(config.statements[i]->tokens[1]);
            handlers_out->push_back(&handler);
        // Add static file with its root directory and base URL
        } else if (config.statements[i]->tokens[0] == "static_serve") {
            http::handler_file handler(config.statements[i]->tokens[2],
                                       config.statements[i]->tokens[1]);
            handlers_out->push_back(&handler);
        }
    }
    
    return handlers_out->size();
}

// Parses all server settings and stores the parsed server setup in 
// server_config out-parm. Returns port number (-1 if no number is found)
int NginxServerConfigParser::parseServerSettings(server_config* 
    server_settings_out) {
    
    server_settings_out->port = -1;
   
    for (size_t i = 0; i < config.statements.size(); i++) {
        if (config.statements[i]->tokens[0] == "port") {
            server_settings_out->port = 
                                  std::stoi(config.statements[i]->tokens[1]);
        }
    }
 
    return server_settings_out->port;
}

// Returns the port number from config_file or -1 if no number is found
//   TODO: Put this in a class as a static method
int port_number(const char* config_file) {
    int port = -1;
    NginxConfigParser config_parser;
    NginxConfig config;
    config_parser.Parse(config_file, &config);
    for (size_t i = 0; i < config.statements.size(); i++) {
        if (config.statements[i]->tokens[0] == "port") {
            port = std::stoi(config.statements[i]->tokens[1]);
        } 
    }
    return port;
}

