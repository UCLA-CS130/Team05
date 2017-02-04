#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>
#include "server.h"
#include "server_config_parser.h"

using boost::asio::ip::tcp;


// Entry point of the program
int main(int argc, char* argv[]) {
    try {
        // Make sure a config file is passed to the program
        if (argc != 2) {
            std::cerr << "Usage: webserver <config_file>" << std::endl;
            return 1;
        }

        // Get the port number from the config file
        int port = port_number(argv[1]);
        if (port == -1) {
            std::cerr << "Missing port <number> in config file" << std::endl;
            return 1;
        }

        // Start the server
        boost::asio::io_service io_service;
        server s(io_service, port);
        printf("Running server on port %d...\n", port);
        io_service.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}

