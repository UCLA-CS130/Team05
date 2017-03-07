#ifndef REQUEST_H
#define REQUEST_H

#include <memory>
#include <string>
#include <vector>


// Represents an HTTP Request
//
// Usage:
//   auto request = Request::Parse(raw_request);
class Request {
public:

    // Parser results from the Parse method
    enum Result {
        good,
        bad,
        indeterminate
    };

    // Parses data from the client. The return value is valid when a
    // complete request has been parsed, nullptr if the data is invalid or
    // indeterminate
    static std::unique_ptr<Request> Parse(const std::string& raw_request);
    
    // Return the result of the request Parse function
    static Result GetParseResult();    

    // Default constructor
    Request();
    
    // Reset the request to fresh state
    void Reset();

    // Returns the value for the given header or the empty string
    std::string GetHeaderValue(const std::string& name) const;

    // Gets the unparsed string that represents the request
    std::string raw_request() const;

    // Gets the method for the request (indicates what is to be performed)
    std::string method() const;

    // Gets the resource identifier for the request
    std::string uri() const;

    // Sets the uri of the request
    void SetUri(const std::string& uri);

    // Returns the path represented by the URI or the empty string if invalid
    std::string path() const;

    // Gets the HTTP version the request was made with
    std::string version() const;

    // Gets the headers of the request
    using Headers = std::vector<std::pair<std::string, std::string> >;
    Headers headers() const;

    // Gets the body of the request
    std::string body() const;

protected:

    // Handles the next character of input to the parser
    Result Consume(char input);

private:

    // State used internally by the parser
    enum {
        _method_start,
        _method,
        _uri,
        _http_version_h,
        _http_version_t_1,
        _http_version_t_2,
        _http_version_p,
        _http_version_slash,
        _http_version_major_start,
        _http_version_major,
        _http_version_minor_start,
        _http_version_minor,
        _expecting_newline_1,
        _header_line_start,
        _header_lws,
        _header_name,
        _space_before_header_value,
        _header_value,
        _expecting_newline_2,
        _expecting_newline_3,
        _body
    } state;

    unsigned long long remaining; // Used to track remaining characters to parse

    std::string raw_request_; // The entire, unparsed request as a string
    std::string method_;      // Indicates what is to be performed
    std::string uri_;         // Identifies the resource for the request
    std::string path_;        // Path represented by the URI
    std::string version_;     // HTTP version the requester is using
    Headers     headers_;     // Headers included in the request
    std::string body_;        // Body of the request
};


#endif // REQUEST_H

