#include "status_handler.h"
#include "server.h"


// Initializes the handler. Returns OK if successful
// uri_prefix is the value in the config file that this handler will run for
// config is the contents of the child block for this handler ONLY
RequestHandler::Status StatusHandler::Init(const std::string& uri_prefix, 
const NginxConfig& config) {
    // The status handler does not do any initializing
    return RequestHandler::OK;
}


// Handles an HTTP request, and generates a response. Returns a response code
// indicating success or failure condition. If ResponseCode is not OK, the
// contents of the response object are undefined, and the server will return
// HTTP code 500
RequestHandler::Status StatusHandler::HandleRequest(const Request& request, 
Response* response) {
    // Get all the info we need from the server and lock
    Server* s = Server::GetInstance();
    s->Lock();
    const std::map<std::string, std::string>& hi = s->GetHandlersByUrl();
    const std::vector<std::pair<std::string, int> >& rh = s->GetRequestHistory();

    // Begin the HTML of our status page
    std::string html = "<html><head><title>Webserver Status</title></head>"
        "<body>\r\n"
        "Total number of request handlers: ";
    html += std::to_string(hi.size());
    html += "\r\n<br><br><table style=\"width:640px\"><tr>"
        "<th align=\"left\">URL Prefix</th>"
        "<th align=\"left\" style=\"width:100px\">Handler Type</th>"
        "</tr>";

    // Write out the table of requests
    for (auto it = hi.begin(); it != hi.end(); ++it) {
        html += "<tr><td>";
        html += it->first;
        html += "</td><td style=\"width:100px\">";
        html += it->second;
        html += "</td></tr>";
    }

    // Continue to the next table
    html += "</table><br><br><br>\r\n"
        "Total number of requests received so far: ";
    html += std::to_string(rh.size());
    html += "\r\n<br><br><table style=\"width:640px\"><tr>"
        "<th align=\"left\">Request URI</th>"
        "<th align=\"left\" style=\"width:100px\">Response Code</th>"
        "</tr>";

    // Write out the table of requests
    for (size_t i = 0; i < rh.size(); i++) {
        html += "<tr><td>";
        html += rh[i].first;
        html += "</td><td style=\"width:100px\">";
        html += std::to_string(rh[i].second);
        html += "</td></tr>";
    }
    
    // Unlock server when we're done
    s->Unlock();

    // Finish the HTML of our status page
    html += "</table></body></html>";
    *response = Response::HtmlResponse(std::move(html));
    return RequestHandler::OK;
}

