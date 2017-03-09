// This file is based on both the Boost HTTP server and Echo example at
//  http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/examples/cpp11_examples.html

#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <thread>
#include <boost/asio.hpp>
#include "request.h"
#include "response.h"
#include "server.h"

using boost::asio::ip::tcp;

// Constructor taking a list of HTTP request handlers
Session::Session(tcp::socket sock,
const std::map<std::string, std::unique_ptr<RequestHandler> >& hndlers) :
handlers(hndlers), socket(std::move(sock)) {

}


// Starts the session between client and server
void Session::start() {
    do_read();
}


// Callback for when a client should have its data read
void Session::do_read() {
    boost::system::error_code ec;
    
    // Read up to max_length bytes from the client then write it back
    std::size_t len = socket.read_some(boost::asio::buffer(buf), ec);
    
    // Check if an error has occurred during the connection
    if (!ec) {
        // No error : parse the data from the client
        std::unique_ptr<Request> request = Request::Parse(std::string(
        buf.data(), len));
        if (Request::GetParseResult() == Request::good) {
            // Get the path for determining what handler were using
            std::string path = request->path();
            if (path.empty()) {
                do_write("(bad request)", Response::DefaultResponse(
                    Response::bad_request));
                return;
            }

            // Get the correct request handler per all possible prefixes
            //  Note: This will loop until it reaches the empty string,
            //  and we are guarenteed the empty string is in the map
            std::string path_prefix = path;
            std::map<std::string, std::unique_ptr<RequestHandler>
                >::const_iterator it = handlers.find(path_prefix);
            while (it == handlers.end()) {
                // For every slash, we check if a handler exists both
                // with the slash and without it before moving on to
                // the next least-nested prefix
                std::size_t slash_pos = path_prefix.find_last_of("/");
                path_prefix = path_prefix.substr(0, slash_pos + 1);
                it = handlers.find(path_prefix);
                if (it == handlers.end()) {
                    path_prefix = path_prefix.substr(0, slash_pos);
                    it = handlers.find(path_prefix);
                }
            }

            // Handle the request or return response code 500 if not OK
            RequestHandler::Status status;
            Response response;
            status = it->second->HandleRequest(*request, &response);
            if (status == RequestHandler::OK) {
                do_write(request->uri(), response);
            } else {
                do_write(request->uri(), Response::DefaultResponse(
                    Response::internal_server_error));
            }
        } else if (Request::GetParseResult() == Request::bad) {
            do_write("(bad request)", Response::DefaultResponse(
                Response::bad_request));
        } else {
            // Indeterminate request, continue reading
            do_read();
        }
    }

    //  Initiate graceful connection closure.
    boost::system::error_code ignored_ec;
    socket.shutdown(tcp::socket::shutdown_both, ignored_ec);
}


// Callback for when a client should be written to
void Session::do_write(const std::string& uri, const Response& res) {
    // Print out the data we're sending to the client
    std::string res_str = res.ToString();
    printf("Sending the following to %s\n",
        socket.remote_endpoint().address().to_string().c_str());
    printf("==========\n");
    printf("%s\n", res_str.c_str());
    printf("==========\n");

    // Track the responses all responses that are sent out
    Server* server = Server::GetInstance();
    server->Lock();
    server->requests.push_back(
        std::make_pair(uri, (int)res.GetStatus()));
    server->Unlock();
    
    // Send the response back to the client and then we're done
    boost::asio::write(socket, boost::asio::buffer(res_str, res_str.size()));
}


// Constructor taking a list of HTTP request handlers
Server::Server(boost::asio::io_service& io_service, int port,
const std::map<std::string, std::unique_ptr<RequestHandler> >& hndlers,
const std::map<std::string, std::string>& hndler_info) :
handlerInfo(hndler_info), handlers(hndlers),
acceptor(io_service, tcp::endpoint(tcp::v4(), port)), socket(io_service) {
    // Start accepting clients as soon as the server instance is created
    do_accept();
}


// Callback for when a client attempts to connect
void Server::do_accept() {
    // Accept clients continuously in other threads than the main thread
    acceptor.accept(socket);
                
    // This session makes a thread per request
    auto session_ptr = std::make_shared<Session>(std::move(socket),
                                                 handlers);
    std::thread request_thread([session_ptr] { 
        session_ptr->start(); 
    });

    // Thread detached to continue processing request independently
    request_thread.detach();

    // Repeatedly do this
    do_accept();
}

