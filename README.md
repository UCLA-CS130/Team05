CS130 - Software Engineering - Winter 2017

## Team 05  
Alfred Lucero  
Brandon Haffen  
Jordan Miller

## Description  
Creating a webserver using Boost with GTest Unit Testing, GCov Test  
Coverage, Bash Integration Testing, and Nginx-inspired parsing of config  
files - modeling the HTTP Restful API  

## Usage  
In order to create the executable and try our webserver, just run "make"  
in the project directory. Our tests can be run via "make test", and test  
coverage can be measured with "make test_gcov". We also implement "make  
clean" which we find necessary after running our tests.  

Our code is all in the root directory of the project folder. It starts  
main.cc, which loads up the config file via our server_config_parser.cc then  
quickly yields most the heavy-duty work to server.cc. Within server.cc,  
our request.cc file parses incoming requests from clients, and then server.cc  
uses the handlers it parsed from the config file and the interface in  
request_handler.h to process these parsed requests. Finally, a response is  
generated using the response.cc interface and forwarded back to the client.  

In order to add a new handler, you will need to first create a header and  
implementation file for it. Then, add the file to be built via the  
Makefile. This is as easy as adding the implementation file to the SRC  
variable. Adding a test is more involved, and requires adding to both the  
TESTEXEC and the GCOVEXEC variables as well as making a *_test and *_gcov  
target for your test. You should use the tests already there to see how to  
do this.  

When creating a handler, you only need to edit your own files in order to  
expose the handler to the entire system. The REGISTER_REQUEST_HANDLER  
macro in your header file will fulfill this duty. You should model your  
header file off of one of ours. We recommend using echo_handler.h for a  
simple example, and static_file_handler.h for a more complete one. All  
that is left after making the header file is to fill your implementation  
with code that takes advantage of our virtual functions Init and  
HandleRequest. Please read our comments and check out our own handlers for  
more information.  

Once you are all done with making your handler, you can use it by running the  
server with a config file that gives your handler a path. Please see  
example_config for an example on how to do that.  

In order to build a compressed image of the server using docker, use the "make  
docker" command. This shrunken image is intended to be used to run the  
webserver on cloud hosts such as Amazon Web Services. Docker must be installed  
prior to making or using the image. In addition, in order to deploy  
your most recent docker image to the AWS instance, run "make deploy",  
which shrinks the image, transfers the file to the AWS instance, kills  
the old server process, and starts a new server with the updated  
docker image.

## Multithreading  
The server begins with a single thread that accepts TCP connections. Any time  
a connection is accepted, the server creates a new thread for that connection.  
This thread reads any data sent and writes back a response. The thread closes  
as soon as it writes back a response or the connection closes. This means that  
the server can respond to many different requests all at the same time.

