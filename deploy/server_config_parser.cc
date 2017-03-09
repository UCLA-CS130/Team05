#include "server_config_parser.h"
#include "server.h"
#include "request_handler.h"
#include <stdio.h>


// Constructor: pass in a config file to be parsed for its contents
NginxServerConfigParser::NginxServerConfigParser(const char* config_file) {
    // Parse the config file into statements and tokens for us to extract data
    config_parser.Parse(config_file, &config);
}


// Parses all server request handlers and stores the parsed
// handlers in a vector out-param. Returns number of request handlers found
int NginxServerConfigParser::ParseRequestHandlers(
std::map<std::string, std::unique_ptr<RequestHandler> >& handlers_out,
std::map<std::string, std::string>& handler_info_out) {
    // Check config file for request handlers to add
    for (size_t i = 0; i < config.statements.size(); i++) {
        // Only attempt to read statements with a first token we care about
        if (config.statements[i]->tokens.size() > 0) {
            if (config.statements[i]->tokens[0] == "path") {
                // Paths must have 2 extra tokens past the first "path" one
                if (config.statements[i]->tokens.size() > 2) {
                    auto handler = RequestHandler::CreateByName(
                        config.statements[i]->tokens[2].c_str());
                    if (handler && config.statements[i]->child_block) {
                        if (handler->Init(config.statements[i]->tokens[1],
                            *(config.statements[i]->child_block)) ==
                            RequestHandler::OK) {
                            handlers_out.insert(std::pair<std::string,
                                std::unique_ptr<RequestHandler> >(
                                config.statements[i]->tokens[1],
                                std::move(std::unique_ptr<RequestHandler>(
                                handler))));
                            handler_info_out[config.statements[i]->tokens[1]] =
                                config.statements[i]->tokens[2];
                        }
                    } else {
                        printf("Handler does not exist (%s)",
                            config.statements[i]->tokens[2].c_str());
                    }
                } else {
                    printf("\"path\" statement in config needs more tokens\n");
                }
            } else if (config.statements[i]->tokens[0] == "default") {
                // Default must have 1 extra tokens past the first "default" one
                if (config.statements[i]->tokens.size() > 1) {
                    auto handler = RequestHandler::CreateByName(
                        config.statements[i]->tokens[1].c_str());
                    if (handler && config.statements[i]->child_block) {
                        if (handler->Init("",
                            *(config.statements[i]->child_block)) ==
                            RequestHandler::OK) {
                            handlers_out.insert(std::pair<std::string,
                                std::unique_ptr<RequestHandler> >("",
                                std::move(std::unique_ptr<RequestHandler>(
                                handler))));
                            handler_info_out["(default)"] =
                                config.statements[i]->tokens[1];
                        }
                    } else {
                        printf("Handler does not exist (%s)",
                            config.statements[i]->tokens[1].c_str());
                    }
                } else {
                    printf("\"default\" statement in config needs more tokens\n");
                }
            }
        }
    }

    // Ensure that a default handler is made
    if (handlers_out.find("") == handlers_out.end()) {
        auto handler = RequestHandler::CreateByName("NotFoundHandler");
        handler->Init("", NginxConfig());
        handlers_out.insert(std::pair<std::string,
            std::unique_ptr<RequestHandler> >("",
            std::move(std::unique_ptr<RequestHandler>(handler))));
        handler_info_out["(default)"] = "NotFoundHandler";
    }

    return handlers_out.size();
}


// Parses all server settings and stores the parsed server setup in 
// server_config out-parm. Returns port number (-1 if no number is found)
int NginxServerConfigParser::ParseServerSettings(
ServerSettings& server_settings_out) {
    server_settings_out.port = -1;

    for (size_t i = 0; i < config.statements.size(); i++) {
        if (config.statements[i]->tokens[0] == "port") {
            if (server_settings_out.port == -1) {
                try {
                    int port = std::stoi(config.statements[i]->tokens[1]);
                    if (port >= 0 && port <= 65535) {
                        server_settings_out.port =
                            std::stoi(config.statements[i]->tokens[1]);
                    }
                } catch (...) {}
            } else {
                printf("Multiple ports found in config file\n");
            }
        }
    }

    return server_settings_out.port;
}

