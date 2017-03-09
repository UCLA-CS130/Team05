#ifndef RESPONSE_H
#define RESPONSE_H

#include <string>
#include <vector>


// Represents an HTTP response
//
// Usage:
//   Response r;
//   r.SetStatus(RESPONSE_200);
//   r.SetBody(...);
//   return r.ToString();
//
// Constructed by the RequestHandler, after which the server should call
// ToString to serialize the request
class Response {
public:

    typedef std::pair<std::string, std::string> Header;

    // HTTP response codes
    enum ResponseCode {
        ok = 200,
        created = 201,
        accepted = 202,
        no_content = 204,
        moved_permanently = 301,
        moved_temporarily = 302,
        not_modified = 304,
        bad_request = 400,
        unauthorized = 401,
        forbidden = 403,
        not_found = 404,
        internal_server_error = 500,
        not_implemented = 501,
        bad_gateway = 502,
        service_unavailable = 503
    };

    // Creates a default response for a given status code
    static Response DefaultResponse(ResponseCode status);

    // Creates a plain text response for the given text
    static Response PlainTextResponse(std::string&& text);

    // Creates a text/html response for the given html
    static Response HtmlResponse(std::string&& html);  

    // Get the headers of the response
    using Headers = std::vector<std::pair<std::string, std::string>>;
    Headers GetHeaders() const;

    // Adds a header to the response
    void AddHeader(const std::string& name, const std::string& value);
    
    // Get the body of the response
    std::string GetBody() const;

    // Sets the body of the response
    void SetBody(const std::string& body);
    
    // Get the status of the response
    ResponseCode GetStatus() const;

    // Sets the status of the repsonse
    void SetStatus(const ResponseCode response_code);
    
    // Converts response to a string representing data to be sent to the client
    std::string ToString() const;

    // Sets the whole response, headers and body
    void SetFullResponse(const std::string& response);

private:

    ResponseCode status; // Status code of the http response
    std::vector<std::pair<std::string, std::string> > headers; // Vector of headers for the http response
    std::string content; // Body of http response
    std::string full_response; // Header + Body of response
};


namespace DefaultResponse {

// Gets default message body for a given status code
std::string ToHtml(Response::ResponseCode status);

} // namespace DefaultResponse

#endif // RESPONSE_H

