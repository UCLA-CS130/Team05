import re
from subprocess import Popen
from subprocess import PIPE
import sys

# The expected output from curl's echo request as a regular expression
curl_output_expected_echo = [
    "GET /echo HTTP/1\\.0\r\n",
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

# We start off with the assumption that the test will succeed
ec = 0

# Open the web server in a subprocess
webserver = Popen(["./webserver", "sample_config"], stdout=PIPE)

# Request an echo from the webserver using curl
curl = Popen(["curl", "-0", "-s", "localhost:1234/echo"], stdout=PIPE)
curl_output = curl.communicate()[0].decode()
sys.stdout.write("\nServer output for integration test 1\n")
sys.stdout.write("==========\n")
sys.stdout.write(curl_output)
sys.stdout.write("==========\n\n")
for string in curl_output_expected_echo:
    pattern = re.compile(string)
    if not pattern.search(curl_output):
        ec = 1
        sys.stdout.write("FAILED to match the following regular expression:\n")
        sys.stdout.write("  " + string + "\n")

# Request a file from the webserver using curl
curl = Popen(["curl", "-0", "-s", "localhost:1234/static/test_file"], stdout=PIPE)
curl_output = curl.communicate()[0].decode()
sys.stdout.write("\nServer output for integration test 2\n")
sys.stdout.write("==========\n")
sys.stdout.write(curl_output)
sys.stdout.write("==========\n\n")
if not curl_output == "TEST\n":
    ec = 1
    sys.stdout.write("FAILED to match the following regular expression:\n")
    sys.stdout.write("  TEST\n")

# Request an image file from the webserver using curl
curl = Popen(["curl", "-0", "-s", "-I", "localhost:1234/static/bunny.jpg"], stdout=PIPE)
curl_output = curl.communicate()[0].decode()
sys.stdout.write("\nServer output for integration test 3\n")
sys.stdout.write("==========\n")
sys.stdout.write(curl_output)
sys.stdout.write("==========\n\n")
for string in curl_output_expected_static_image:
    pattern = re.compile(string)
    if not pattern.search(curl_output):
        ec = 1
        sys.stdout.write("FAILED to match the following regular expression:\n")
        sys.stdout.write("  " + string + "\n")

# Request a file from the webserver using curl that does not exist
curl = Popen(["curl", "-0", "-s", "localhost:1234/static/doesnotexist"], stdout=PIPE)
curl_output = curl.communicate()[0].decode()
sys.stdout.write("\nServer output for integration test 4\n")
sys.stdout.write("==========\n")
sys.stdout.write(curl_output)
sys.stdout.write("\n==========\n\n")
if not curl_output == "<html><head><title>Not Found</title></head><body><h1>404 Not Found</h1></body></html>":
    ec = 1
    sys.stdout.write("FAILED to match the following regular expression:\n")
    sys.stdout.write("  <html><head><title>Not Found</title></head><body><h1>404 Not Found</h1></body></html>\n")

# Close the webserver
webserver.terminate()

# Return 0 if the test succeeded or some other value on failure
if ec == 0:
    sys.stdout.write("INTEGRATION TEST SUCCEEDED\n")
    sys.exit(0)
else:
    sys.stdout.write("INTEGRATION TEST FAILED\n")
    sys.exit(ec)

