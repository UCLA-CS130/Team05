import re
from subprocess import Popen
from subprocess import PIPE
import socket
import sys

# The expected output from curl's echo request as a regular expression
curl_output_expected_echo = [
    "GET /echo HTTP/1\\.0\r\n",
    "Host: localhost:\\d+\r\n",
    "User-Agent: curl/\\d+\\.\\d+\\.\\d+\r\n",
    "Accept: \\*/\\*\r\n"
]

# The expected output from curl's echo request as a regular expression
curl_output_expected_echo2 = [
    "GET /echo/ HTTP/1\\.0\r\n",
    "Host: localhost:\\d+\r\n",
    "User-Agent: curl/\\d+\\.\\d+\\.\\d+\r\n",
    "Accept: \\*/\\*\r\n"
]

# The expected output from curl's echo request as a regular expression
curl_output_expected_static_image = [
    "HTTP/1\\.0 200 OK\r\n",
    "Content-Length: \\d+\r\n",
    "Content-Type: image/jpeg\r\n"
]

# The expected output from the multithreading test
expected_multithreaded_output = (
    "HTTP/1.0 200 OK\r\n"
    "Content-Length: 22\r\n"
    "Content-Type: text/plain\r\n"
    "\r\n"
    "GET /echo HTTP/1.0\r\n"
    "\r\n"
)

# We start off with the assumption that the test will succeed
ec = 0



if (len(sys.argv) < 2):
    sys.stdout.write("Wrong number args\n")
    sys.exit(1)

n = int(sys.argv[1])

sys.stdout.write("Number of sockets to create: ")
sys.stdout.write(str(n))
sys.stdout.write("\n")

# Open the web server in a subprocess
webserver = Popen(["./webserver", "example_config"], stdout=PIPE)



curl = Popen(["curl", "-0", "-s", "localhost:2020/echo"], stdout=PIPE)



# Open the web server in a subprocess
# webserver = Popen(["./webserver", "example_config"], stdout=PIPE)

sockets = []
# Test the multithreading capabilities of the webserver

for i in xrange(n):

    s= socket.socket()
    sockets.append(s)
    s.connect(("localhost", 2020))
    if i < (n-1):
        sockets[i].send("GET /echo HT")
    else:
        sockets[i].send("GET /echo HTTP/1.0\r\n\r\n")
        sockets[i].settimeout(1)
        multithreaded_output = sockets[i].recv(1024)
        
        sys.stdout.write("\nServer output for integration test 7 - multithreading\n")
        sys.stdout.write("==========\n")
        sys.stdout.write(multithreaded_output)
        sys.stdout.write("\n==========\n\n")
        if not multithreaded_output == expected_multithreaded_output:
            ec = 1
            sys.stdout.write("FAILED to match the following expression:\n")
            sys.stdout.write(expected_multithreaded_output)

for i in xrange(n):
    sockets[i].close()



# Close the webserver
webserver.terminate()

# Return 0 if the test succeeded or some other value on failure
if ec == 0:
    sys.stdout.write("INTEGRATION TEST SUCCEEDED\n")
    sys.exit(0)
else:
    sys.stdout.write("INTEGRATION TEST FAILED\n")
    sys.exit(ec)

