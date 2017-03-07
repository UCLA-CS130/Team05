#include "gtest/gtest.h"
#include "echo_handler.h"
#include "request.h"
#include "response.h"


TEST(EchoHandlerTest, Echo) {
    std::string test_string = "GET /echo HTTP/1.0\r\n"
                              "Host: localhost:1234\r\n"
                              "\r\n";

    std::unique_ptr<Request> request = Request::Parse(test_string);

    EXPECT_TRUE(request != NULL);

    NginxConfig config;

    EchoHandler h;
    EXPECT_EQ(RequestHandler::Status::OK, h.Init("/echo", config));

    Response response;
    EXPECT_EQ(RequestHandler::Status::OK, h.HandleRequest(*request, &response));

    EXPECT_EQ(test_string, response.GetBody());

    using Headers = std::vector<std::pair<std::string, std::string>>;
    Headers headers = response.GetHeaders();

    ASSERT_EQ(2, headers.size());
    EXPECT_EQ("Content-Type", response.GetHeaders()[1].first);
    EXPECT_EQ("text/plain", response.GetHeaders()[1].second);

}