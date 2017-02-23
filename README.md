CS130 - Software Engineering - Winter 2017

Team 05:  
    Alfred Lucero  
    Brandon Haffen  
    Jordan Miller

Description:  
    Creating a webserver using Boost with GTest Unit Testing, GCov Test  
    Coverage, Bash Integration Testing, and Nginx-inspired parsing of config  
    files - modeling the HTTP Restful API  

Usage:  
    In order to create the executable and try our webserver, just run "make"  
    in the project directory. Our tests can be run via "make test", and test  
    coverage can be measured with "make test_gcov". We also implement "make  
    clean" which we find necessary after running our tests.  

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

