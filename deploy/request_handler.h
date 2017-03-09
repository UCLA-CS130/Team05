// Based off of https://github.com/jfarrell468/registerer

#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include <map>
#include <memory>
#include <string>
#include "config_parser.h"
#include "request.h"
#include "response.h"


// Represents the parent of all request handlers. Implementations should expect
// to be long lived and created at server construction
class RequestHandler {
public:

    // Request handler return codes
    enum Status {
        OK = 0,
        Error = 1
    };

    // Registers a request handler by name
    static RequestHandler* CreateByName(const char* type);
  
    // Initializes the handler. Returns OK if successful
    // uri_prefix is the value in the config file that this handler will run for
    // config is the contents of the child block for this handler ONLY
    virtual Status Init(const std::string& uri_prefix,
                        const NginxConfig& config) = 0;

    // Handles an HTTP request, and generates a response. Returns a response code
    // indicating success or failure condition. If ResponseCode is not OK, the
    // contents of the response object are undefined, and the server will return
    // HTTP code 500
    virtual Status HandleRequest(const Request& request,
                                 Response* response) = 0;
};


// Notes:
// * The trick here is that you can declare an object at file scope, but you
//   can't do anything else, such as set a map key. But you can get around this
//   by creating a class that does work in its constructor.
// * request_handler_builders must be a pointer. Otherwise, it won't necessarily
//   exist when the RequestHandlerRegisterer constructor gets called.
extern std::map<std::string, RequestHandler* (*)(void)>* request_handler_builders;
template<typename T>
class RequestHandlerRegisterer {
public:
    RequestHandlerRegisterer(const std::string& type) {
        if (request_handler_builders == nullptr) {
            request_handler_builders = new std::map<std::string, RequestHandler* (*)(void)>;
        }
        (*request_handler_builders)[type] = RequestHandlerRegisterer::Create;
    }
    static RequestHandler* Create() {
        return new T;
    }
};

#define REGISTER_REQUEST_HANDLER(ClassName) \
  static RequestHandlerRegisterer<ClassName> ClassName##__registerer(#ClassName)

#endif // REQUEST_HANDLER_H

