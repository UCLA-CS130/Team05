#ifndef REVERSE_PROXY_HANDLER_H
#define REVERSE_PROXY_HANDLER_H

#include "request_handler.h"


// Interacts with clients from the Internet at-large
// as if this server was the one serving up the
// content requested, rather than the remote servers
// to which it actually reaches out.
//
// See: https://github.com/UCLA-CS130/Mr.-Robot-et-al./issues/62

// We use this in reverse_proxy_handler_test.cc
class ReverseProxyHandlerTest;

// Retrieves a file from the server's file system
class ReverseProxyHandler : public RequestHandler {
public:

    // We use friend so that ReverseProxyHandlerTest can access private variables for testing
    friend class ReverseProxyHandlerTest;

    // Initializes the handler. Returns OK if successful
    // uri_prefix is the value in the config file that this handler will run for
    // config is the contents of the child block for this handler ONLY
    virtual Status Init(const std::string& uri_prefix,
                        const NginxConfig& config);

    // Handles an HTTP request, and generates a response. Returns a response code
    // indicating success or failure condition. If ResponseCode is not OK, the
    // contents of the response object are undefined, and the server will return
    // HTTP code 500
    virtual Status HandleRequest(const Request& request, Response* response);

private:

    std::string original_uri_prefix;
    std::string remote_host;
    std::string remote_port;

    // Helper for 302-redirects
    // Returns the value of the Location header,
    // e.g., "www.exmple.com" for "Location: www.example.com"
    std::string getLocationHeaderValue(const std::string& response_buffer_string);

    // Helper for parsing response code from origin responses
    std::string getRemoteResponseCode(const std::string& response_buffer_string);

    // Helper for sending proxy requests
    // Returns the string representation of the response received from the remote_host
    // Origin is the remote_host that is the original source of the content
    // we're serving on their behalf
    std::string sendRequestToOrigin(Request request, std::string remote_uri);

    // Helper for editing the path of relative URIs
    // When HTML elements require resources with a path relative to their origin, for
    // example a stylesheet for www.foobar.com/home with the path: 'href="../css/style.css', we
    // need to change the '../' to '/home/'.
    void rerouteRelativeUris(std::string& response_body);

    // // Function for verifying certificate from remote server is https is used
    // //   Taken from: http://www.boost.org/doc/libs/1_47_0/doc/html/boost_asio/example/ssl/client.cpp
    // bool verifyCertificate(bool preverified,
    //   boost::asio::ssl::verify_context& ctx);
};

REGISTER_REQUEST_HANDLER(ReverseProxyHandler);

#endif  // REVERSE_PROXY_HANDLER_H
