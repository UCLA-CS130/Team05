#include "gtest/gtest.h"
#include "server_config_parser.h"


class PortParseTest : public ::testing::Test {
protected:

    bool getPortNumber(std::string config_contents) {
        int fd;
        char name[] = "/tmp/fileXXXXXX";

        // Creates a temporary file using name, replacing the last 6 X's 
        fd = mkstemp(name);

        // Write the config_contents into the temp file
        write(fd, config_contents.c_str(), config_contents.size());

        port = port_number(name);

        // Delete the temporary file
        remove(name);

        return (port != -1);
    }

    int port;
};


TEST_F(PortParseTest, ValidPortConfig) {
    ASSERT_TRUE(getPortNumber("port 80800;"));
    EXPECT_EQ(80800, port);
}


TEST_F(PortParseTest, InvalidPortConfig) {
    EXPECT_FALSE(getPortNumber("Port 80800;"));
}

