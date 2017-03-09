#ifndef SERVER_H
#define SERVER_H

#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>
#include <mutex>
#include "request.h"
#include "request_handler.h"
#include "response.h"
#include "echo_handler.h"
#include "static_file_handler.h"

using boost::asio::ip::tcp;


// Represents a session between the server and a client
class Session : public std::enable_shared_from_this<Session> {
public:

    // Constructor taking a list of HTTP request handlers
    Session(tcp::socket sock,
    const std::map<std::string, std::unique_ptr<RequestHandler> >& hndlers);

    // Starts the session between client and server
    void start();

private:

    // Max length of a single read from the client
    enum { max_length = 8192 };

    // Callback for when a client should have its data read
    void do_read();

    // Callback for when a client should be written to
    void do_write(const std::string& uri, const Response& res);

    // Reference to the vector of request handlers
    const std::map<std::string, std::unique_ptr<RequestHandler> >& handlers;

    std::array<char, max_length> buf; // Buffer used when reading client data
    Request     request;              // Structure for storing client requests
    tcp::socket socket;               // Used to represent a client
};


// Represents the server
class Server  {
public:

    // Returns the instance of the server using the singleton pattern
    static Server* GetInstance() {
        static Server* instance = (Server*) malloc(sizeof(Server));
        return instance;
    }

    // Begins running the server with the given port and handlers
    static void Run(int port,
    const std::map<std::string, std::unique_ptr<RequestHandler> >& hndlers,
    const std::map<std::string, std::string>& hndler_info) {
        boost::asio::io_service io_service;
        Server* server = GetInstance();
        new (server) Server(io_service, port, std::move(hndlers),
            std::move(hndler_info));
        io_service.run();
    }

    // Gets a map of URLs to strings representing what handler they're using
    const std::map<std::string, std::string>& GetHandlersByUrl() const {
        return handlerInfo;
    }

    // Gets a vector of all the requests received so far
    //  Note: the data contained in the pair is URI and response code
    const std::vector<std::pair<std::string, int> >& GetRequestHistory() const {
        return requests;
    }

    // Lock a critical section
    void Lock() {
        mtx.lock();
    }

    // Unlock a critical section
    void Unlock() {
        mtx.unlock();
    }

protected:

    // Session is a friend class so it can edit the below var
    friend class Session;

    // Vector containing all the previous requests to the server
    std::vector<std::pair<std::string, int> > requests;

    // Map of URL prefixes to their handlers (updated by ServerConfigParser)
    const std::map<std::string, std::string>& handlerInfo;

private:

    // Constructor taking a list of HTTP request handlers
    Server(boost::asio::io_service& io_service, int port,
    const std::map<std::string, std::unique_ptr<RequestHandler> >& hndlers,
    const std::map<std::string, std::string>& hndler_info);

    // Callback for when a client attempts to connect
    void do_accept();

    // Vector of request handlers - used across multiple sessions/threads
    const std::map<std::string, std::unique_ptr<RequestHandler> >& handlers;

    tcp::acceptor acceptor;    // Used in boost.asio to take in new clients
    tcp::socket   socket;      // Used in boost.asio to represent clients
    std::mutex    mtx;         // Used to lock status handler critical section
};

#endif // SERVER_H

