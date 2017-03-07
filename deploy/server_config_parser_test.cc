#include "gtest/gtest.h"
#include "server_config_parser.h"


class PortParseTest : public ::testing::Test {
protected:

    // Test to check if server parser can read in port correctly
    //  Returns true if the port is not -1
    bool GetPortNumber(std::string config_contents) {
        int fd;
        char name[] = "/tmp/fileXXXXXX";

        // Creates a temporary file using name, replacing the last 6 X's 
        fd = mkstemp(name);

        // Write the config_contents into the temp file
        write(fd, config_contents.c_str(), config_contents.size());

        // Initialize server parser
        NginxServerConfigParser server_parser(name);
      
        // Get the port number
        port = server_parser.ParseServerSettings(server_settings);

        // Delete the temporary file
        remove(name);

        return (port != -1);
    }

    int port;
    ServerSettings server_settings;
};


TEST_F(PortParseTest, ValidPortConfig) {
    ASSERT_TRUE(GetPortNumber("port 8080;"));
    EXPECT_EQ(8080, port);
}


TEST_F(PortParseTest, InvalidPortConfig) {
    EXPECT_FALSE(GetPortNumber("pOrT 8080;"));
    EXPECT_FALSE(GetPortNumber("port 80800;"));
}


class HandlerParseTest : public ::testing::Test {
protected:

    // Test to check the number of handlers parsed from config file
    //  Returns true iff at least one handler is parsed
    int GetNumberOfHandlersParsed(std::string config_contents) {
        int fd;
        char name[] = "/tmp/fileXXXXXX";

        // Creates a temporary file using name, replacing the last 6 X's
        fd = mkstemp(name);

        // Write the config_contents into the temp file
        write(fd, config_contents.c_str(), config_contents.size());
        
        // Initialize server parser
        NginxServerConfigParser server_parser(name);
        
        // Parse the echo and static file request handlers from the config file
        return server_parser.ParseRequestHandlers(handlers, handler_info);
    }

    std::map<std::string, std::unique_ptr<RequestHandler> > handlers;
    std::map<std::string, std::string> handler_info;
};


TEST_F(HandlerParseTest, NoHandlersConfig) {
    EXPECT_EQ(GetNumberOfHandlersParsed("no /handlers;"), 1);
    EXPECT_EQ(handlers.size(), 1);
    EXPECT_EQ(handler_info.size(), 1);
    EXPECT_TRUE(handlers.find("") != handlers.end());
    EXPECT_EQ(handler_info["(default)"], "NotFoundHandler");
}


TEST_F(HandlerParseTest, DefaultHandlerConfig) {
    EXPECT_EQ(GetNumberOfHandlersParsed("default EchoHandler {};"), 1);
    EXPECT_EQ(handlers.size(), 1);
    EXPECT_TRUE(handlers.find("") != handlers.end());
    EXPECT_EQ(handler_info.size(), 1);
    EXPECT_EQ(handler_info["(default)"], "EchoHandler");
}


TEST_F(HandlerParseTest, SomeHandlersConfig) {
    EXPECT_EQ(GetNumberOfHandlersParsed("path /echo EchoHandler {}\n"
        "path / StaticFileHandler {\n"
        "  root ./;\n"
        "}"), 3);
    EXPECT_EQ(handlers.size(), 3);
    EXPECT_EQ(handler_info.size(), 3);
    EXPECT_TRUE(handlers.find("/echo") != handlers.end());
    EXPECT_EQ(handler_info["/echo"], "EchoHandler");
    EXPECT_TRUE(handlers.find("/") != handlers.end());
    EXPECT_EQ(handler_info["/"], "StaticFileHandler");
    EXPECT_TRUE(handlers.find("") != handlers.end());
    EXPECT_EQ(handler_info["(default)"], "NotFoundHandler");
}


TEST_F(HandlerParseTest, ImproperHandlerConfig) {
    EXPECT_EQ(GetNumberOfHandlersParsed("path /echo StaticFileHandler"), 1);
    EXPECT_TRUE(handlers.find("") != handlers.end());
    EXPECT_EQ(handler_info["(default)"], "NotFoundHandler");
}

