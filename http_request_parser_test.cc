#include "gtest/gtest.h"
#include "http_request.h"
#include "http_request_parser.h"


class HttpRequestParserTest : public ::testing::Test {
protected:

    http::request_parser parser;
};


TEST_F(HttpRequestParserTest, DecodeUrl) {
    std::string in;
    std::string out;

    in = "Foo%20bar+baz";
    EXPECT_TRUE(http::request_parser::decode_url(in, out));
    EXPECT_EQ("Foo bar baz", out);

    in = "Foo%4";
    EXPECT_FALSE(http::request_parser::decode_url(in, out));

    in = "Foo%iooyay";
    EXPECT_FALSE(http::request_parser::decode_url(in, out));
}


TEST_F(HttpRequestParserTest, Parse) {
    http::request request;
    std::tuple<http::request_parser::result, std::string::iterator> result;
    std::string test_string;

    // First subtest is for a correct parse of an HTTP request
    test_string = "GET /echo%20world HTTP/1.0\r\n"
                  "Host: localhost:1234\r\n"
                  "User-Agent: test\r\n"
                  "Accept: */*\r\n"
                  "\r\nRANDOMGARBAGE";
    result = parser.parse(request, test_string.begin(), test_string.end());
    EXPECT_EQ(http::request_parser::good, std::get<0>(result));
    EXPECT_EQ(test_string.end() - 13, std::get<1>(result));
    EXPECT_EQ("GET", request.method);
    EXPECT_EQ("/echo%20world", request.uri);
    http::header headers[3];
    headers[0].name = "Host";       headers[0].value = "localhost:1234";
    headers[1].name = "User-Agent"; headers[1].value = "test";
    headers[2].name = "Accept";     headers[2].value = "*/*";
    EXPECT_EQ("/echo world", request.path);
    for (int i = 0; i < 3; i++) {
        EXPECT_EQ(headers[i].name,  request.headers[i].name);
        EXPECT_EQ(headers[i].value, request.headers[i].value);
    }
    EXPECT_EQ(test_string.substr(0, test_string.size() - 13), request.as_string);

    // Second subtest checks an indeterminate request
    parser.reset();
    test_string = "GET /echo%20world HT";
    result = parser.parse(request, test_string.begin(), test_string.end());
    EXPECT_EQ(http::request_parser::indeterminate, std::get<0>(result));
    EXPECT_EQ(test_string.end(), std::get<1>(result));

    // Last subtest checks to make sure the parser fails if request ill-formed
    parser.reset();
    test_string = "This is just some\r\n"
                  "random text\r\n";
    result = parser.parse(request, test_string.begin(), test_string.end());
    EXPECT_EQ(http::request_parser::bad, std::get<0>(result));
    EXPECT_EQ('u', (*std::get<1>(result)));
}

