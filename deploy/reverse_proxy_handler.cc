#include <boost/algorithm/string/replace.hpp>
#include <boost/asio.hpp>
#include <boost/tokenizer.hpp>
#include <iostream>
#include "reverse_proxy_handler.h"

using boost::asio::ip::tcp;

// Helper function for tokenizing HTTP requests
// From: https://github.com/UCLA-CS130/Mr.-Robot-et-al./blob/c9b064c68cd4bc1ae6b5c012db59eae9cb8b946d/request.cc#L7
boost::tokenizer<boost::char_separator<char>>
tokenGenerator(const std::string s, const char* sep) {
    // Tokenize the rest_lines
    boost::char_separator<char> separator(sep);
    boost::tokenizer<boost::char_separator<char>> tokens(s, separator);
    return tokens;
}

// Initializes the handler. Returns OK if successful
// uri_prefix is the value in the config file that this handler will run for
// config is the contents of the child block for this handler ONLY
RequestHandler::Status ReverseProxyHandler::Init(const std::string& uri_prefix,
    const NginxConfig& config) {
    // Check config child block for settings
    for (size_t i = 0; i < config.statements.size(); i++) {
        // Only attempt to read statements with a first token we care about
        if (config.statements[i]->tokens.size() > 0) {
            if (config.statements[i]->tokens[0] == "remote_host") {
              // remote_host must have 2 tokens
              if (config.statements[i]->tokens.size() > 1) {
                  remote_host = config.statements[i]->tokens[1];
              } else {
                  printf("\"remote_host\" statement in config needs more tokens\n");
              }
          }
          if (config.statements[i]->tokens[0] == "remote_port") {
              // remote_port must have 2 tokens
              if (config.statements[i]->tokens.size() > 1) {
                  remote_port = config.statements[i]->tokens[1];
              } else {
                  printf("\"remote_port\" statement in config needs more tokens\n");
              }
          }
      }
    }
    if (!remote_host.empty() && !remote_port.empty()) {
        original_uri_prefix = uri_prefix;
        return RequestHandler::OK;
    } else {
        printf("Missing remote_port or remote_host provided to ReverseProxyHandler\n");
        return RequestHandler::Error;
    }
}

// Helper for 302-redirects
// Returns the value of the Location header,
// e.g., "www.exmple.com" for "Location: www.example.com"
std::string ReverseProxyHandler::getLocationHeaderValue(const std::string& response_buffer_string) {
    // Grab the Location header line
    int start_location = response_buffer_string.find("Location:");
    int end_location = response_buffer_string.find_first_of("\r\n", start_location);
    std::string location_line = response_buffer_string.substr(start_location, end_location);

    boost::tokenizer<boost::char_separator<char>> tokens
        = tokenGenerator(location_line, " ");

    // Split the location line to get the value
    std::string location_value;
    int i = 0;
    for (auto cur_token = tokens.begin();
         cur_token != tokens.end();
         cur_token++,
         i++) {
        if (i == 1) {
            location_value = *cur_token;
            break;
        }
    }

    return location_value;
}

// Helper for parsing response code from origin responses
std::string ReverseProxyHandler::getRemoteResponseCode(const std::string& response_buffer_string) {
    // Response has the form:
    // HTTP/1.0 200 OK
    // Content-Length: 3497
    // Content-Type: text/plain
    //
    // <body contents>
    //
    // We check just the first line
    size_t end_first_line = response_buffer_string.find_first_of("\r\n");
    std::string first_line = response_buffer_string.substr(0, end_first_line);

    // TODO: Move this response-code checking into a helper function

    // Tokenize and check second token for ResponseCode
    // Based on: https://github.com/UCLA-CS130/Mr.-Robot-et-al./blob/c9b064c68cd4bc1ae6b5c012db59eae9cb8b946d/request.cc#L46
    boost::tokenizer<boost::char_separator<char>> tokens
        = tokenGenerator(first_line, " ");

    // Handle response code from remote_host.
    // remote_host's ResponseCode => our ResponseCode cases:
    //
    // 200 => 200
    // 302 => fetch-loop to 200 or 404 (upon not-found or max-retries)
    // 404 => 404
    // 500 => 404
    std::string return_response_code;
    int i = 0;
    for (auto cur_token = tokens.begin();
         cur_token != tokens.end();
         cur_token++,
         i++) {
        if (i == 1) {
            std::string remote_response_code = *cur_token;
            // std::cerr << "remote_response_code: " << remote_response_code << std::endl;

            if (remote_response_code == "200") {
                return_response_code = "200";
            } else if (remote_response_code == "302") {
                return_response_code = "302";
            } else if (remote_response_code == "404") {
                return_response_code = "404";
            } else if (remote_response_code == "500") {
                return_response_code = "404";
            } else {
                printf("ReverseProxyHandler does not understand the remote_host's ResponseCode\n");
                return_response_code = "500";
            }
            break;
        }
    }

    return return_response_code;
}

// Helper for sending proxy requests
// Returns the string representation of the response received from the remote_host
// Origin is the remote_host that is the original source of the content
// we're serving on their behalf
std::string ReverseProxyHandler::sendRequestToOrigin(Request request, std::string remote_uri) {
    boost::asio::io_service io_service;
    tcp::socket socket(io_service);
    tcp::resolver resolver(io_service);

    // Construct new request to send to remote host
    // Example: Modify request from: /reverse_proxy/static/file1.txt
    // to:                           /static/file1.txt
    remote_uri.erase(0, remote_uri.find("/", 1));

    if (remote_uri.empty()) {
      remote_uri = "/";
    }
    request.SetUri(remote_uri);

    // Use resolver to handle DNS lookup if query is not an IP address
    boost::system::error_code ec;
    std::string new_response = "";
    bool got_302 = false;
    do {
      // Avoid "host not found" error
      // See: https://stackoverflow.com/questions/12542460/boost-asio-host-not-found-authorative
      tcp::resolver::query query(remote_host,
                                 remote_port,
                                 boost::asio::ip::resolver_query_base::numeric_service);

      // A hostname could resolve to multiple endpoints to try;
      // connect() will try all of them
      // See: http://www.boost.org/doc/libs/1_62_0/boost/asio/connect.hpp
      boost::asio::connect(socket, resolver.resolve(query), ec);
      if (ec == boost::asio::error::not_found) {
          printf("Reverse proxy connection attempt to remote host failed. Check hostname and port number.\n");
          return "RequestHandler::Error";
      }

      // std::cerr << "Got past connecting to remote_host!" << std::endl;

      std::string remote_request = request.raw_request();
      size_t host_pos = remote_request.find("Host");
      size_t host_end = remote_request.find("\r\n", host_pos);
      size_t host_len = host_end - host_pos;
      remote_request.replace(host_pos, host_len, "Host: " + remote_host);
      // std::cerr << "---------Sending remote_request-----------\n"
                // << remote_request
                // << "----------End of remote_request----------\n";
      socket.send(boost::asio::buffer(remote_request));

      const int MAX_BUFFER_LENGTH = 1024;
      size_t bytes_received = 0;
      do {
        char response_buffer[MAX_BUFFER_LENGTH];
        bytes_received = socket.receive(boost::asio::buffer(response_buffer), {}, ec);
        if (!ec) {
          new_response.append(response_buffer, response_buffer + bytes_received);
        }
        // std::cerr << "bytes_received: " << bytes_received << "    ec: " << ec << std::endl;
      } while (!ec);

      if (new_response.find("HTTP/1.1 302 Found") != std::string::npos) {
        got_302 = true;
        size_t location = new_response.find("Location");
        size_t www = new_response.find("www", location);
        size_t slash = new_response.find("/", www);
        remote_host = new_response.substr(www, slash - www);
        new_response = "";
        // std::cout << "Redirected: setting remote_host to " << remote_host << std::endl;
      }
      else {
        got_302 = false;
      }
      // std::cout << "Got_302? " << got_302 << std::endl;
    } while (got_302);

    // Checking ec set when reading response from remote host
    switch (ec.value()) {
      case boost::asio::error::eof:
      case boost::system::errc::success:
        // std::cerr << "remote_host's response: \n" << new_response << std::endl;
        break;
      default:
        // std::cerr << "Error reading from remote_host, error code: " << ec << std::endl;
        return "RequestHandler::Error";
    }

    return new_response;
}

// Helper for editing the path of relative URIs
// When HTML elements require resources with a path relative to their origin, for
// example a stylesheet for www.foobar.com/home which has the path:
// 'href="../css/style.css"', we need to change the '="../"' to '="/home/../"'.
void ReverseProxyHandler::rerouteRelativeUris(std::string& response_body) {
    if (original_uri_prefix != "/") {
        std::string new_href = "href=\"" + original_uri_prefix + "/";
        boost::replace_all(response_body, "href=\"/", new_href);

        std::string new_src = "src=\"" + original_uri_prefix + "/";
        boost::replace_all(response_body, "src=\"/", new_src);
    }
}

// Handles an HTTP request, and generates a response. Returns a response code
// indicating success or failure condition. If ResponseCode is not OK, the
// contents of the response object are undefined, and the server will return
// HTTP code 500
RequestHandler::Status ReverseProxyHandler::HandleRequest(const Request& request,
    Response* response) {

    std::string response_buffer_string = sendRequestToOrigin(request, request.uri());

    if (response_buffer_string == "RequestHandler::Error") {
        response->SetStatus(Response::not_found);
        return RequestHandler::Error;
    }

    std::string return_response_code = getRemoteResponseCode(response_buffer_string);
    // std::cerr << "return_response_code: " << return_response_code << std::endl;

    if (return_response_code == "200") {
        response->SetStatus(Response::ok);
    } else if (return_response_code == "404") {
        response->SetStatus(Response::not_found);
    } else if (return_response_code == "500") {
        response->SetStatus(Response::internal_server_error);
    }

    response->SetFullResponse(response_buffer_string);

    return RequestHandler::OK;
}
