#include "gtest/gtest.h"
#include "server_config_parser.h"


class PortParseTest : public ::testing::Test {
protected:
    // Test to check if server parser can read in port correctly
    // Returns true if the port is not -1
    bool getPortNumber(std::string config_contents) {
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

    short port;
    server_config server_settings;

};


TEST_F(PortParseTest, ValidPortConfig) {
    ASSERT_TRUE(getPortNumber("port 80800;"));
    EXPECT_EQ(80800, port);
}


TEST_F(PortParseTest, InvalidPortConfig) {
    EXPECT_FALSE(getPortNumber("Port 80800;"));
}

class HandlerParseTest : public ::testing::Test {
protected:
    // Test to check the number of handlers parsed from config file
    // Returns true iff at least one handler is parsed
    bool getNumberOfHandlersParsed(std::string config_contents) {
        int fd;
        char name[] = "/tmp/fileXXXXXX";

        // Creates a temporary file using name, replacing the last 6 X's
        fd = mkstemp(name);

        // Write the config_contents into the temp file
        write(fd, config_contents.c_str(), config_contents.size());
        
        // Initialize server parser
        NginxServerConfigParser server_parser(name);
        
        // Parse the echo and static file request handlers from the config file
        server_parser.ParseRequestHandlers(handlers);
        
        num_parsed_handlers = handlers.size();
        return num_parsed_handlers;
    }

    std::vector<std::unique_ptr<http::handler> > handlers;
    int num_parsed_handlers;
    
};

TEST_F(HandlerParseTest, NoHandlersConfig) {
    ASSERT_FALSE(getNumberOfHandlersParsed("no /handlers;"));
    EXPECT_EQ(num_parsed_handlers, 0);
}

TEST_F(HandlerParseTest, SomeHandlersConfig) {
    ASSERT_TRUE(getNumberOfHandlersParsed("echo /echo;\nstatic_serve /static_serve ./;"));
    EXPECT_EQ(num_parsed_handlers, 2);
}
