#include "gtest/gtest.h"
#include "http_handler_echo.h"
#include "http_request.h"
#include "http_response.h"


class HttpHandlerEchoTest : public ::testing::Test {
protected:
    void form_request(std::string path) {
        r.path = path; 
        r.method = "GET";
        r.uri = "/";       
    }
    
    http::request r;
   

};


TEST_F(HttpHandlerEchoTest, SimpleEcho) {

	
    http::handler_echo hf("/echo");
    form_request("/echo/Makefile");

    http::response res1 = hf.handle_request(r);

    http::response res2 = http::response::plain_text_response(std::string(r.as_string));

    EXPECT_EQ(res1.headers[0].name, res1.headers[0].name);
    EXPECT_EQ(res2.headers[0].value, res1.headers[0].value);
    EXPECT_EQ(res2.headers[1].name, res1.headers[1].name);
	EXPECT_EQ(res2.headers[1].value, res1.headers[1].value);
	EXPECT_EQ(res2.content, res1.content);
	EXPECT_EQ(res2.status, res1.status);


    

	
}