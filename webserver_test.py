import re
from subprocess import Popen
from subprocess import PIPE
import socket
import sys

def outputChecker(curl, curl_expected):
    curl_output = curl.communicate()[0]
    sys.stdout.write("\nServer output for integration test\n")
    sys.stdout.write("==========\n")
    sys.stdout.write(curl_output)
    sys.stdout.write("==========\n\n")
    for string in curl_expected:
        pattern = re.compile(string)
        if not pattern.search(curl_output):
            return 1
            sys.stdout.write("FAILED to match the following regular expression:\n")
            sys.stdout.write("  " + string + "\n")
    return 0

# The expected output from curl's echo request as a regular expression
curl_output_expected_echo = [
    "GET /echo HTTP/1\\.0\r\n"
    # "Host: localhost:\\d+\r\n",
    # "User-Agent: curl/\\d+\\.\\d+\\.\\d+\r\n",
    # "Accept: \\*/\\*\r\n"
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

# Simple test to see if we get an OK from server with remote host being google.com
curl_output_expected_https_reverse_proxy = [
    "HTTP/1\\.0 200 OK\r\n"
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

# Open the web server in a subprocess
webserver1 = Popen(["./webserver", "example_config"], stdout=PIPE)

# Open another web server in a subprocess
webserver2 = Popen(["./webserver", "proxy_test_config"], stdout=PIPE)

# Open yet another web server in a subprocess
webserver3 = Popen(["./webserver", "https_proxy_config"], stdout=PIPE)

# Request an echo from the webserver using curl
curl = Popen(["curl", "-0", "-s", "localhost:2020/echo"], stdout=PIPE)
ec += outputChecker(curl, curl_output_expected_echo)

# Request an echo from the webserver using curl
curl = Popen(["curl", "-0", "-s", "localhost:2020/echo/"], stdout=PIPE)
ec += outputChecker(curl, curl_output_expected_echo2)

# Request a file from the webserver using curl
curl = Popen(["curl", "-0", "-s", "localhost:2020/test_file"], stdout=PIPE)
curl_output = curl.communicate()[0].decode()
sys.stdout.write("\nServer output for integration test\n")
sys.stdout.write("==========\n")
sys.stdout.write(curl_output)
sys.stdout.write("==========\n\n")
if not curl_output == "TEST\n":
    ec += 1
    sys.stdout.write("FAILED to match the following regular expression:\n")
    sys.stdout.write("  TEST\n")

# Request an image file from the webserver using curl
curl = Popen(["curl", "-0", "-s", "-I", "localhost:2020/bunny.jpg"], stdout=PIPE)
ec += outputChecker(curl, curl_output_expected_static_image)

# Request a file from the webserver using curl that does not exist
curl = Popen(["curl", "-0", "-s", "localhost:2020/doesnotexist"], stdout=PIPE)
curl_output = curl.communicate()[0].decode()
sys.stdout.write("\nServer output for integration test\n")
sys.stdout.write("==========\n")
sys.stdout.write(curl_output)
sys.stdout.write("\n==========\n\n")
if not curl_output == "<html><head><title>Not Found</title></head><body><h1>404 Not Found</h1></body></html>":
    ec += 1
    sys.stdout.write("FAILED to match the following regular expression:\n")
    sys.stdout.write("  <html><head><title>Not Found</title></head><body><h1>404 Not Found</h1></body></html>\n")
    
# Test the multithreading capabilities of the webserver
s1 = socket.socket()
s1.connect(("localhost", 2020))
s1.send("GET /echo HT")
s2 = socket.socket()
s2.connect(("localhost", 2020))
s2.send("GET /echo HTTP/1.0\r\n\r\n")
s2.settimeout(1)
multithreaded_output = s2.recv(1024)
s1.close()
s2.close()
sys.stdout.write("\nServer output for integration test\n")
sys.stdout.write("==========\n")
sys.stdout.write(multithreaded_output)
sys.stdout.write("\n==========\n\n")
if not multithreaded_output == expected_multithreaded_output:
    ec += 1
    sys.stdout.write("FAILED to match the following expression:\n")
    sys.stdout.write(expected_multithreaded_output)

# Request echo response from the reverse proxy webserver
curl = Popen(["curl", "-0", "-s", "localhost:4242/reverse_proxy/echo"], stdout=PIPE)
ec += outputChecker(curl, curl_output_expected_echo)

# Request a file from the  reverse proxy webserver
curl = Popen(["curl", "-0", "-s", "localhost:4242/reverse_proxy/test_file"], stdout=PIPE)
curl_output = curl.communicate()[0].decode()
sys.stdout.write("\nServer output for integration test\n")
sys.stdout.write("==========\n")
sys.stdout.write(curl_output)
sys.stdout.write("==========\n\n")
if not curl_output == "TEST\n":
    ec += 1
    sys.stdout.write("FAILED to match the following regular expression:\n")
    sys.stdout.write("  TEST\n")

# Request an image from the  reverse proxy webserver
curl = Popen(["curl", "-0", "-s", "-o", "proxy_bunny", "localhost:4242/reverse_proxy/bunny.jpg"], stdout=PIPE)
curl.wait()
diff = Popen(["cmp", "bunny.jpg", "proxy_bunny"], stdout=PIPE)
if (diff.communicate()[0].decode() != ""):
    ec += 1
    sys.stdout.write("FAILED to download matching copy of bunny.jpg via reverse-proxy\n")

# Request ucla.edu from reverse proxy webserver for 302 testing
curl = Popen(["curl", "-0", "-s", "localhost:4242/redirect"], stdout=PIPE)
curl_output = curl.communicate()[0].decode("utf-8")
# Note: ucla.edu results in a 302 Found redirect to www.ucla.edu
curl_expected = Popen(["curl", "-0", "-s", "www.ucla.edu"], stdout=PIPE)
curl_output_expected = curl_expected.communicate()[0].decode("utf-8")
if (curl_output != curl_output_expected):
    ec += 1
    sys.stdout.write("FAILED to redirect to ucla.edu via reverse-proxy\n")

# Close the webserver
webserver1.terminate()
webserver2.terminate()

# Check if we get a valid response from https reverse proxy to google.com
curl = Popen(["curl", "-0", "-s", "-I", "localhost:4343"], stdout=PIPE)
ec += outputChecker(curl, curl_output_expected_https_reverse_proxy)

# Get page through proxy and straight from google.com and compare results
curl = Popen(["curl", "-0", "-s", "localhost:4343"], stdout=PIPE)
# Grab first 900 characters because script associated with timestamp makes results differ
curl_output = curl.communicate()[0].decode("windows-1252")[:900]
curl_expected = Popen(["curl", "-0", "-s", "https://www.google.com"], stdout=PIPE)
curl_output_expected = curl_expected.communicate()[0].decode("windows-1252")[:900]
if (curl_output != curl_output_expected):
    ec += 1
    sys.stdout.write("FAILED to match google.com via reverse-proxy\n")


# Close the webserver
webserver3.terminate()

# Return 0 if the test succeeded or some other value on failure
if ec == 0:
    sys.stdout.write("INTEGRATION TESTS SUCCEEDED\n")
    sys.exit(0)
else:
    sys.stdout.write("INTEGRATION TESTS FAILED\n")
    sys.exit(ec)
