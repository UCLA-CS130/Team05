#include "server_config_parser.h"
#include "http_handler_echo.h"
#include "http_handler_file.h"


// Constructor: pass in a config file to be parsed for its contents
NginxServerConfigParser::NginxServerConfigParser(const char* config_file) {
    // Parse the config file into statements and tokens for us to extract data
    config_parser.Parse(config_file, &config);
}


// Parses all server echo/static file request handlers and stores the parsed
// handlers in a vector out-param. Returns number of request handlers found
int NginxServerConfigParser::ParseRequestHandlers(
std::vector<std::unique_ptr<http::handler> >& handlers_out) {
    // Check config file for echo/static file requests to add
    for (size_t i = 0; i < config.statements.size(); i++) {
        // Add echo handler with its base URL
        if (config.statements[i]->tokens[0] == "echo") {
            // Form should be: echo /base_url
            if (config.statements[i]->tokens.size() == 2 && 
                config.statements[i]->tokens[1][0] == '/') {
                handlers_out.push_back(std::move(std::unique_ptr<http::handler>(
                    new http::handler_echo(config.statements[i]->tokens[1]))));
            }
        // Add static file with its root directory and base URL
        } else if (config.statements[i]->tokens[0] == "static") {
            // Form should be: static /base_url root_directory/
            if (config.statements[i]->tokens.size() == 3 &&
                config.statements[i]->tokens[1][0] == '/') {
                handlers_out.push_back(std::move(std::unique_ptr<http::handler>(
                    new http::handler_file(config.statements[i]->tokens[2],
                                           config.statements[i]->tokens[1]))));
            }
        }
    }

    return handlers_out.size();
}


// Parses all server settings and stores the parsed server setup in 
// server_config out-parm. Returns port number (-1 if no number is found)
int NginxServerConfigParser::ParseServerSettings(
server_config& server_settings_out) {
    server_settings_out.port = -1;

    for (size_t i = 0; i < config.statements.size(); i++) {
        if (config.statements[i]->tokens[0] == "port") {
            server_settings_out.port =
                                  std::stoi(config.statements[i]->tokens[1]);
        }
    }

    return server_settings_out.port;
}
