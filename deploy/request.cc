#include "request.h"
#include <memory>
#include <sstream>
#include <thread>

// These are local variables per thread 
thread_local Request::Result result = Request::indeterminate;
thread_local Request local_request;

// Default constructor
Request::Request() : state(_method_start) {
}


// Reset the request to fresh state
void Request::Reset() {
    raw_request_ = std::string();
    method_ = std::string();
    uri_ = std::string();
    path_ = std::string();
    version_ = std::string();
    headers_ = Headers();
    body_ = std::string();
    state = _method_start;
}


// Parses data from the client. The return value is valid when a
// complete request has been parsed, nullptr if the data is invalid or
// indeterminate
std::unique_ptr<Request> Request::Parse(const std::string& raw_request) {
    result = indeterminate;
    std::string::const_iterator begin   = raw_request.begin();
    std::string::const_iterator end     = raw_request.end();
    while (begin != end) {
        result = local_request.Consume(*begin++);
        if (result == good) {
            auto r = std::unique_ptr<Request>(new Request(local_request));
            local_request.Reset();
            return r;
        } else if (result == bad) {
            local_request.Reset();
            return std::unique_ptr<Request>(nullptr);
        }
    }
    
    // Must be indeterminate request
    return std::unique_ptr<Request>(nullptr);
}


// Return the result of the request Parse function 
Request::Result Request::GetParseResult() {
    return result;
}


// Returns the value for the given header or the empty string
std::string Request::GetHeaderValue(const std::string& name) const {
    for (std::size_t i = 0; i < headers_.size(); i++) {
        if (headers_[i].first == name) {
            return headers_[i].second;
        }
    }
    return std::string();
}


// Gets the unparsed string that represents the request
std::string Request::raw_request() const {
    return raw_request_;
}


// Gets the method for the request (indicates what is to be performed)
std::string Request::method() const {
    return method_;
}


// Gets the resource identifier for the request
std::string Request::uri() const {
    return uri_;
}

// Sets the uri for the request
void Request::SetUri(const std::string& uri) {
  uri_ = uri;
  size_t uri_pos = raw_request_.find(" ") + 1;
  size_t uri_end = raw_request_.find(" ", uri_pos);
  size_t uri_len = uri_end - uri_pos;
  raw_request_.replace(uri_pos, uri_len, uri);
}

// Returns the path represented by the URI or the empty string if invalid
std::string Request::path() const {
    return path_;
}


// Gets the HTTP version the request was made with
std::string Request::version() const {
    return version_;
}


// Gets the headers of the request
using Headers = std::vector<std::pair<std::string, std::string> >;
Headers Request::headers() const {
    return headers_;
}


// Gets the body of the request
std::string Request::body() const {
    return body_;
}


// Helper function for the parser that checks if c is a character
static bool is_char(int c) {
    return c >= 0 && c <= 127;
}


// Helper function for the parser that checks if c is a control character
static bool is_ctl(int c) {
    return (c >= 0 && c <= 31) || (c == 127);
}


// Helper function for the parser that checks if c is a HTTP tspecial
static bool is_tspecial(int c) {
    switch (c) {
    case '(': case ')': case '<': case '>': case '@':
    case ',': case ';': case ':': case '\\': case '"':
    case '/': case '[': case ']': case '?': case '=':
    case '{': case '}': case ' ': case '\t':
        return true;
    default:
        return false;
    }
}


// Helper function for the parser that checks if c is a digit
static bool is_digit(int c) {
    return c >= '0' && c <= '9';
}


Request::Result Request::Consume(char input) {
    raw_request_ += input;
    switch (state) {
    case _method_start:
        if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
            return bad;
        } else {
            state = _method;
            method_.push_back(input);
            return indeterminate;
        }
    case _method:
        if (input == ' ') {
            state = _uri;
            return indeterminate;
        } else if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
            return bad;
        } else {
            method_.push_back(input);
            return indeterminate;
        }
    case _uri:
        if (input == ' ') {
            state = _http_version_h;

            // Now that we have the URI, infer the path from it
            path_.reserve(uri_.size());
            for (std::size_t i = 0; i < uri_.size(); ++i) {
                if (uri_[i] == '%') {
                    if (i + 3 <= uri_.size()) {
                        int value = 0;
                        std::istringstream is(uri_.substr(i + 1, 2));
                        if (is >> std::hex >> value) {
                            path_ += static_cast<char>(value);
                            i += 2;
                        } else {
                            path_ = std::string();
                            return indeterminate;
                        }
                    } else {
                        path_ = std::string();
                        return indeterminate;
                    }
                } else if (uri_[i] == '+') {
                    path_ += ' ';
                } else {
                    path_ += uri_[i];
                }
            }

            // Ensure that the path does not go backwards in the directory
            if (path_.find("..") != std::string::npos) path_ = std::string();

            // If path ends in slash (i.e. is a directory) then add "index.html"
            else if (path_[path_.size() - 1] == '/') path_ += "index.html";

            return indeterminate;
        } else if (is_ctl(input)) {
            return bad;
        } else {
            uri_.push_back(input);
            return indeterminate;
        }
    case _http_version_h:
        if (input == 'H') {
            state = _http_version_t_1;
            version_.push_back('H');
            return indeterminate;
        } else {
            return bad;
        }
    case _http_version_t_1:
        if (input == 'T') {
            state = _http_version_t_2;
            version_.push_back('T');
            return indeterminate;
        } else {
            return bad;
        }
    case _http_version_t_2:
        if (input == 'T') {
            state = _http_version_p;
            version_.push_back('T');
            return indeterminate;
        } else {
            return bad;
        }
    case _http_version_p:
        if (input == 'P') {
            state = _http_version_slash;
            version_.push_back('P');
            return indeterminate;
        } else {
            return bad;
        }
    case _http_version_slash:
        if (input == '/') {
            state = _http_version_major_start;
            version_.push_back('/');
            return indeterminate;
        } else {
            return bad;
        }
    case _http_version_major_start:
        if (is_digit(input)) {
            state = _http_version_major;
            version_.push_back(input);
            return indeterminate;
        } else {
            return bad;
        }
    case _http_version_major:
        if (input == '.') {
            state = _http_version_minor_start;
            version_.push_back('.');
            return indeterminate;
        } else if (is_digit(input)) {
            version_.push_back(input);
            return indeterminate;
        } else {
            return bad;
        }
    case _http_version_minor_start:
        if (is_digit(input)) {
            state = _http_version_minor;
            version_.push_back(input);
            return indeterminate;
        } else {
            return bad;
        }
    case _http_version_minor:
        if (input == '\r') {
            state = _expecting_newline_1;
            return indeterminate;
        } else if (is_digit(input)) {
            version_.push_back(input);
            return indeterminate;
        } else {
            return bad;
        }
    case _expecting_newline_1:
        if (input == '\n') {
            state = _header_line_start;
            return indeterminate;
        } else {
            return bad;
        }
    case _header_line_start:
        if (input == '\r') {
            state = _expecting_newline_3;
            return indeterminate;
        } else if (!headers_.empty() && (input == ' ' || input == '\t')) {
            state = _header_lws;
            return indeterminate;
        } else if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
            return bad;
        } else {
            headers_.push_back(std::pair<std::string, std::string>());
            headers_.back().first.push_back(input);
            state = _header_name;
            return indeterminate;
        }
    case _header_lws:
        if (input == '\r') {
            state = _expecting_newline_2;
            return indeterminate;
        } else if (input == ' ' || input == '\t') {
            return indeterminate;
        } else if (is_ctl(input)) {
            return bad;
        } else {
            state = _header_value;
            headers_.back().second.push_back(input);
            return indeterminate;
        }
    case _header_name:
        if (input == ':') {
            state = _space_before_header_value;
            return indeterminate;
        } else if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
            return bad;
        } else {
            headers_.back().first.push_back(input);
            return indeterminate;
        }
    case _space_before_header_value:
        if (input == ' ') {
            state = _header_value;
            return indeterminate;
        } else {
            return bad;
        }
    case _header_value:
        if (input == '\r') {
            state = _expecting_newline_2;
            return indeterminate;
        } else if (is_ctl(input)) {
            return bad;
        } else {
            headers_.back().second.push_back(input);
            return indeterminate;
        }
    case _expecting_newline_2:
        if (input == '\n') {
            state = _header_line_start;
            return indeterminate;
        } else {
            return bad;
        }
    case _expecting_newline_3:
        if (input == '\n') {
            try {
                remaining = std::stoull(GetHeaderValue("Content-Length"));
                if (remaining > 0) {
                    state = _body;
                    return indeterminate;
                } else {
                    return good;
                }
            } catch (...) {
                return good;
            }
        } else {
            return bad;
        }
    case _body:
        body_.push_back(input);
        remaining--;
        if (remaining > 0) { 
            return indeterminate;
        } else {
            return good;
        }
    default:
        return bad;
    }
}

