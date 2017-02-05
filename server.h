#ifndef SERVER_H
#define SERVER_H

#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>
#include "http_request.h"
#include "http_request_parser.h"
#include "http_response.h"
#include "http_handler_echo.h"
#include "http_handler_file.h"

using boost::asio::ip::tcp;


// Represents a session between the server and a client
class session : public std::enable_shared_from_this<session> {
public:

    // Constructor
    session(tcp::socket sock, const std::vector<http::handler *>& handlers);

    // Starts the session between client and server
    void start();

private:

    // Max length of a single read from the client
    enum { max_length = 8192 };

    // Callback for when a client should have its data read
    void do_read();

    // Callback for when a client should be written to
    void do_write(const http::response& res);

    std::array<char, max_length> buf; // Buffer used when reading client data
    http::request_parser parser;      // Parser for incoming client requests
    http::request        request;     // Structure for storing client requests
    tcp::socket          socket;      // Used to represent a client
    const std::vector<http::handler *>& handlers; // Vector of request handlers
};


// Represents the server
class server  {
public:

    // Constructor
    server(boost::asio::io_service& io_service, short port, std::vector<http::handler *> handlers);

private:

    // Callback for when a client attempts to connect
    void do_accept();

    tcp::acceptor acceptor; // Used in boost.asio to take in new clients
    tcp::socket   socket;   // Used in boost.asio to represent clients
    std::vector<http::handler *> handlers; // Vector of request handlers, going to be used across multiple threads
};

#endif // SERVER_H

