import re
from subprocess import Popen
from subprocess import PIPE
import sys

# The expected output from curl as a regular expression
curl_output_expected = [
    "GET /echo HTTP/1\\.0\r\n",
    "Host: localhost:\\d+\r\n",
    "User-Agent: curl/\\d+\\.\\d+\\.\\d+\r\n",
    "Accept: \\*/\\*\r\n"
]

# We start off with the assumption that the test will succeed
ec = 0

# Open the web server in a subprocess
webserver = Popen(["./webserver", "port_config"], stdout=PIPE)

# Query the web server using curl
curl = Popen(["curl", "-0", "-s", "localhost:1234/echo"], stdout=PIPE)
curl_output = curl.communicate()[0].decode()
sys.stdout.write("\nActual output from server\n")
sys.stdout.write("==========\n")
sys.stdout.write(curl_output)
sys.stdout.write("==========\n\n")
for string in curl_output_expected:
    pattern = re.compile(string)
    if not pattern.search(curl_output):
        ec = 1
        sys.stdout.write("FAILED to match the following regular expression:\n")
        sys.stdout.write("  " + string + "\n")

# Close the webserver
webserver.terminate()

# Return 0 if the test succeeded or some other value on failure
if ec == 0:
    sys.stdout.write("INTEGRATION TEST SUCCEEDED\n")
    sys.exit(0)
else:
    sys.stdout.write("INTEGRATION TEST FAILED\n")
    sys.exit(ec)

