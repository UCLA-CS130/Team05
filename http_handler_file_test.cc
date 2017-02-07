#include "gtest/gtest.h"
#include "http_handler_file.h"
#include "http_request.h"
#include "http_response.h"




class HttpHandlerFileTest : public ::testing::Test {
protected:
    void form_request(std::string path) {
        r.path = path;        
    }
    
    http::request r;

};

TEST_F(HttpHandlerFileTest, FileExists) {

    int fd;
    char name[] = "/tmp/fileXXXXXX";


    std::string text_contents = "foo bar\n bar foo";

    // Creates a temporary file using name, replacing the last 6 X's 
    fd = mkstemp(name);

    // Write the config_contents into the temp file
    write(fd, text_contents.c_str(), text_contents.size());

    form_request("/static1" + std::string(name));
    http::handler_file hf("/", "/static1");


    http::response res = hf.handle_request(r);


    //Delete the file
    remove(name);


    EXPECT_EQ("Content-Length", res.headers[0].name);
    EXPECT_EQ(std::to_string(text_contents.size()), res.headers[0].value);
    EXPECT_EQ("Content-Type", res.headers[1].name);
    EXPECT_EQ("text/plain", res.headers[1].value);
    EXPECT_EQ(text_contents, res.content);
    EXPECT_EQ(http::response::status_code::ok, res.status);

    
}

TEST_F(HttpHandlerFileTest, WrongHandlerCalled) {

    int fd;
    char name[] = "/tmp/fileXXXXXX";


    std::string text_contents = "foo bar\n bar foo";

    // Creates a temporary file using name, replacing the last 6 X's 
    fd = mkstemp(name);

    // Write the config_contents into the temp file
    write(fd, text_contents.c_str(), text_contents.size());

    form_request("/static11" + std::string(name));
    http::handler_file hf("/", "/static1");


    http::response res = hf.handle_request(r);

    // Delete the file
    remove(name);

    EXPECT_EQ("Content-Length", res.headers[0].name);
    EXPECT_EQ("Content-Type", res.headers[1].name);
    EXPECT_EQ("text/html", res.headers[1].value);
    EXPECT_EQ(http::response::status_code::internal_server_error, res.status);
    
}


TEST_F(HttpHandlerFileTest, NoFileAskedFor) {

    form_request("/static1");
    http::handler_file hf("/", "/static1");


    http::response res = hf.handle_request(r);


    EXPECT_EQ("Content-Length", res.headers[0].name);
    EXPECT_EQ("Content-Type", res.headers[1].name);
    EXPECT_EQ(http::response::status_code::not_found, res.status);

}


TEST_F(HttpHandlerFileTest, FileDoesNotExit) {

    form_request("/static1/filethatdoesntexist" );
    http::handler_file hf("/", "/static1");


    http::response res = hf.handle_request(r);


    EXPECT_EQ("Content-Length", res.headers[0].name);
    EXPECT_EQ("Content-Type", res.headers[1].name);
    EXPECT_EQ(http::response::status_code::not_found, res.status);

}

TEST_F(HttpHandlerFileTest, InvalidBaseUrl) {

    form_request("");
    http::handler_file hf("/", "/static1");

    http::response res = hf.handle_request(r);

    EXPECT_EQ(http::response::status_code::internal_server_error, res.status);

    form_request("Makefile");

    res = hf.handle_request(r);
    EXPECT_EQ(http::response::status_code::internal_server_error, res.status);

}





