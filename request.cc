#include "request.h"
#include <sstream>


// Default constructor
Request::Request() : state(_method_start) {
}


// Returns the value for the given header or the empty string
std::string Request::FindHeaderValue(const std::string& name) const {
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


// Returns true if out contains the URI converted to a file system path
bool Request::path(std::string& out) const {
    out.clear();
    out.reserve(uri_.size());
    for (std::size_t i = 0; i < uri_.size(); ++i) {
        if (uri_[i] == '%') {
            if (i + 3 <= uri_.size()) {
                int value = 0;
                std::istringstream is(uri_.substr(i + 1, 2));
                if (is >> std::hex >> value) {
                    out += static_cast<char>(value);
                    i += 2;
                } else {
                    return false;
                }
            } else {
                return false;
            }
        } else if (uri_[i] == '+') {
            out += ' ';
        } else {
            out += uri_[i];
        }
    }

    // Ensure that the path does not go backwards in the directory structure
    if (out.find("..") != std::string::npos) return false;

    // If path ends in slash (i.e. is a directory) then add "index.html"
    if (out[out.size() - 1] == '/') out += "index.html";

    return true;
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
                remaining = std::stoull(FindHeaderValue("Content-Length"));
                if (remaining > 0) {
                    state = _body;
                    return indeterminate;
                } else {
                    return good;
                }
            } catch (...) {
                return bad;
            }
        } else {
            return bad;
        }
    case _body:
        if (remaining > 0) {
            body_.push_back(input);
            remaining--;
        } else {
            return good;
        }
    default:
        return bad;
    }
}
