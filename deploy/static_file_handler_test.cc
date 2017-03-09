#include "gtest/gtest.h"
#include "static_file_handler.h"
#include "request.h"
#include "response.h"


TEST(StaticFileHandlerTest, FileExists) {

    int fd; int fd_config;
    NginxConfig config;
    NginxConfigParser parser;
    char name[] = "/tmp/fileXXXXXX";
    char name_config[] = "/tmp/configXXXXXX";


    std::string text_contents = "foo bar\n bar foo";
    std::string config_contents = "root /;";

    // Creates a temporary file using name, replacing the last 6 X's 
    fd = mkstemp(name);

    // Write the text_contents into the temp file
    write(fd, text_contents.c_str(), text_contents.size());


    fd_config = mkstemp(name_config);
    write(fd_config, config_contents.c_str(), config_contents.size());

    std::string test_string = "GET /static/" + std::string(name) +
                              " HTTP/1.0\r\n" +
                              "Host: localhost:1234\r\n" +
                              "\r\n";

    std::unique_ptr<Request> request = Request::Parse(test_string);

    EXPECT_TRUE(request != NULL);

    bool parse_status = parser.Parse(name_config, &config);

    // Delete the file
    remove(name_config);
   
    EXPECT_TRUE(parse_status);

    StaticFileHandler h;
    EXPECT_EQ(RequestHandler::Status::OK, h.Init("/static", config));

    Response response;
    EXPECT_EQ(RequestHandler::Status::OK, h.HandleRequest(*request, &response));


    // Delete the file
    remove(name);
    
    using Headers = std::vector<std::pair<std::string, std::string>>;
    Headers headers = response.GetHeaders();

    ASSERT_EQ(2, headers.size());
    EXPECT_EQ("Content-Length", response.GetHeaders()[0].first);
    EXPECT_EQ(std::to_string(text_contents.size()), response.GetHeaders()[0].second);
    EXPECT_EQ("Content-Type", response.GetHeaders()[1].first);
    EXPECT_EQ("text/plain", response.GetHeaders()[1].second);
    EXPECT_EQ(text_contents, response.GetBody());
    EXPECT_EQ(Response::ResponseCode::ok, response.GetStatus());
   
}

TEST(StaticFileHandlerTest, NoFileExists) {
    int fd; int fd_config;
    NginxConfig config;
    NginxConfigParser parser;
    
    char name_config[] = "/tmp/configXXXXXX";


    
    std::string config_contents = "root /;";

    


    fd_config = mkstemp(name_config);
    write(fd_config, config_contents.c_str(), config_contents.size());

    std::string test_string = "GET /static/NOTEXIST" 
                              " HTTP/1.0\r\n" 
                              "Host: localhost:1234\r\n" 
                              "\r\n";

    std::unique_ptr<Request> request = Request::Parse(test_string);

    EXPECT_TRUE(request != NULL);

    bool parse_status = parser.Parse(name_config, &config);

    // Delete the file
    remove(name_config);
   
    EXPECT_TRUE(parse_status);

    StaticFileHandler h;
    EXPECT_EQ(RequestHandler::Status::OK, h.Init("/static", config));

    Response response;
    EXPECT_EQ(RequestHandler::Status::OK, h.HandleRequest(*request, &response));

   
    EXPECT_EQ(Response::ResponseCode::not_found, response.GetStatus());

}

