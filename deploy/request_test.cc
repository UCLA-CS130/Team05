#include "gtest/gtest.h"
#include "request.h"



TEST(RequestParser, ValidRequest) {

    std::string test_string = "GET /echo%20world HTTP/1.0\r\n"
                              "Host: localhost:1234\r\n"
                              "User-Agent: test\r\n"
                              "Accept: */*\r\n"
                              "\r\n";

    std::unique_ptr<Request> request = Request::Parse(test_string);

    ASSERT_TRUE(request != NULL);
    
    EXPECT_EQ(Request::GetParseResult(), Request::good);    
    
    EXPECT_EQ(test_string, request->raw_request());

    EXPECT_EQ("GET", request->method());

    EXPECT_EQ("/echo world", request->path());

    EXPECT_EQ("/echo%20world", request->uri());  

    EXPECT_EQ("HTTP/1.0", request->version());

    std::vector<std::pair<std::string, std::string> > headers;
    headers = request->headers();
    ASSERT_EQ(3, headers.size()) << "There should be 3 headers";
    EXPECT_EQ("Host", headers[0].first);
    EXPECT_EQ("localhost:1234", headers[0].second);

    EXPECT_EQ("User-Agent", headers[1].first);
    EXPECT_EQ("test", headers[1].second);

    EXPECT_EQ("Accept", headers[2].first);
    EXPECT_EQ("*/*", headers[2].second);

}


TEST(RequestParser, ValidBodyRequest) {

    std::string test_string = "GET /echo%20world HTTP/1.0\r\n"
                              "Host: localhost:1234\r\n"
                              "User-Agent: test\r\n"
                              "Content-Length: 6\r\n"
                              "Accept: */*\r\n"
                              "\r\nFOOBAR";

    std::unique_ptr<Request> request = Request::Parse(test_string);

    ASSERT_TRUE(request != NULL);
    
    EXPECT_EQ(Request::GetParseResult(), Request::good);

    EXPECT_EQ(test_string, request->raw_request());

    EXPECT_EQ("GET", request->method());

    EXPECT_EQ("/echo world", request->path());

    EXPECT_EQ("/echo%20world", request->uri());  

    EXPECT_EQ("HTTP/1.0", request->version());

    std::vector<std::pair<std::string, std::string> > headers;
    headers = request->headers();
    ASSERT_EQ(4, headers.size()) << "There should be 4 headers";
    EXPECT_EQ("Host", headers[0].first);
    EXPECT_EQ("localhost:1234", headers[0].second);

    EXPECT_EQ("User-Agent", headers[1].first);
    EXPECT_EQ("test", headers[1].second);

    EXPECT_EQ("Accept", headers[3].first);
    EXPECT_EQ("*/*", headers[3].second);

    ASSERT_EQ("Content-Length", headers[2].first);
    ASSERT_EQ("6", headers[2].second);

    EXPECT_EQ("FOOBAR", request->body());

}


TEST(RequestParser, IndeterminateRequest) {
    
    std::string test_string = "GET /echo HT";

    std::unique_ptr<Request> request = Request::Parse(test_string);

    ASSERT_TRUE(request == NULL);
    
    EXPECT_EQ(Request::GetParseResult(), Request::indeterminate);

}


TEST(RequestParser, BadRequest) { 

    std::string test_string = "This is just some\r\n"
                              "random text\r\n";

    std::unique_ptr<Request> request = Request::Parse(test_string);

    ASSERT_TRUE(request == NULL);

    EXPECT_EQ(Request::GetParseResult(), Request::bad);
}
