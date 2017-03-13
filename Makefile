# Name of the executable
TARGET=webserver

# Files to be deployed with the webserver for use by it
DEPLOYFILES=example_config bunny.jpg chatbox.htm markdown.md *.so

# Test executables
TESTEXEC=config_parser_test response_test server_config_parser_test request_test static_file_handler_test echo_handler_test not_found_handler_test reverse_proxy_handler_test
GCOVEXEC=config_parser_gcov response_gcov server_config_parser_gcov request_gcov static_file_handler_gcov echo_handler_gcov not_found_handler_gcov reverse_proxy_handler_gcov

# GoogleTest directory and output files
GTEST_DIR=googletest/googletest
GTEST_FILES=libgtest.a gtest-all.o

# Gcov flags and files
GCOVFLAGS=-fprofile-arcs -ftest-coverage
GCOVFILES=*.gcno *.gcda *.gcov

# Compiler flags
CXXFLAGS+= -std=c++11 -pthread -Wall -Werror -ILuaJIT-2.0.4/include/luajit-2.0

# Linker flags
LDFLAGS+= -static-libgcc -static-libstdc++ -Wl,-rpath '-Wl,$$ORIGIN' \
-L./LuaJIT-2.0.4/src -lluajit -pthread -lssl -lcrypto -Wl,-Bstatic -lboost_system \
-lboost_regex -ldl 

# Test flags
TESTFLAGS=-std=c++11 -isystem ${GTEST_DIR}/include -pthread \
-ILuaJIT-2.0.4/include/luajit-2.0

# Test files, e.g., downloaded files from integration tests
TEST_FILES=proxy_bunny

# Source files
SRC=server.cc config_parser.cc response.cc \
server_config_parser.cc request.cc echo_handler.cc \
static_file_handler.cc request_handler.cc \
not_found_handler.cc status_handler.cc reverse_proxy_handler.cc \
cpp-markdown/markdown.cpp cpp-markdown/markdown-tokens.cpp

.PHONY: clean clean_target gcov test test_gcov test_setup deploy docker

$(TARGET): clean_target sqlite lua
	$(CXX) -o $@ main.cc $(SRC) $(CXXFLAGS) $(LDFLAGS)

lua:
	cd LuaJIT-2.0.4 && make install PREFIX=$(shell cd LuaJIT-2.0.4 && pwd)
	cp LuaJIT-2.0.4/lib/libluajit-5.1.so.2 .
	cp LuaJIT-2.0.4/lib/libluajit-5.1.so.2.0.4 .

sqlite:
	bash fix-timestamps.sh
	cd sqlite-autoconf-3170000 && ./configure --prefix=$(shell cd \
	sqlite-autoconf-3170000 && pwd) && make install
	cp sqlite-autoconf-3170000/lib/libsqlite3.so .
	cp sqlite-autoconf-3170000/lib/libsqlite3.so.0 .
	cp sqlite-autoconf-3170000/lib/libsqlite3.so.0.8.6 .

clean_target:
	$(RM) $(TARGET)

clean: clean_target
	$(RM) $(GCOVFILES) $(TESTEXEC) $(GTEST_FILES) *_gcov.txt
	$(RM) $(TEST_FILES)

test_gcov: $(GCOVEXEC)

test_setup:
	g++ $(TESTFLAGS) -I${GTEST_DIR} -c ${GTEST_DIR}/src/gtest-all.cc
	ar -rv libgtest.a gtest-all.o

test: $(TARGET) $(TESTEXEC) integration_test

integration_test: $(TARGET)
	python webserver_test.py

config_parser_test: test_setup config_parser.cc config_parser_test.cc
	g++ $(GCOVFLAGS) $(TESTFLAGS) config_parser_test.cc config_parser.cc ${GTEST_DIR}/src/gtest_main.cc libgtest.a -o $@ $(LDFLAGS)
	./$@

config_parser_gcov: config_parser_test
	gcov -r config_parser.cc > config_parser_gcov.txt

response_test: test_setup response.cc response_test.cc
	g++ $(GCOVFLAGS) $(TESTFLAGS) response_test.cc response.cc ${GTEST_DIR}/src/gtest_main.cc libgtest.a -o $@ $(LDFLAGS)
	./$@

response_gcov: response_test
	gcov -r response.cc > response_gcov.txt

request_test: test_setup request.cc request_test.cc
	g++ $(GCOVFLAGS) $(TESTFLAGS) request_test.cc request.cc ${GTEST_DIR}/src/gtest_main.cc libgtest.a -o $@ $(LDFLAGS)
	./$@

request_gcov: request_test
	gcov -r request.cc > request_gcov.txt

static_file_handler_test: test_setup static_file_handler.cc static_file_handler_test.cc
	g++ $(GCOVFLAGS) $(TESTFLAGS) static_file_handler_test.cc static_file_handler.cc request.cc response.cc not_found_handler.cc request_handler.cc config_parser.cc cpp-markdown/markdown.cpp cpp-markdown/markdown-tokens.cpp ${GTEST_DIR}/src/gtest_main.cc libgtest.a -o $@ $(LDFLAGS)
	./$@

static_file_handler_gcov: static_file_handler_test
	gcov -r static_file_handler.cc > static_file_handler_gcov.txt

echo_handler_test: test_setup echo_handler.cc echo_handler_test.cc
	g++ $(GCOVFLAGS) $(TESTFLAGS) echo_handler_test.cc echo_handler.cc request.cc response.cc request_handler.cc config_parser.cc ${GTEST_DIR}/src/gtest_main.cc libgtest.a -o $@ $(LDFLAGS)
	./$@

echo_handler_gcov: echo_handler_test
	gcov -r echo_handler.cc > echo_handler_gcov.txt

not_found_handler_test: test_setup not_found_handler.cc not_found_handler_test.cc
	g++ $(GCOVFLAGS) $(TESTFLAGS) not_found_handler_test.cc not_found_handler.cc request.cc response.cc request_handler.cc config_parser.cc ${GTEST_DIR}/src/gtest_main.cc libgtest.a -o $@ $(LDFLAGS)
	./$@

not_found_handler_gcov: not_found_handler_test
	gcov -r not_found_handler.cc > not_found_handler_gcov.txt

server_config_parser_test: test_setup server_config_parser.cc server_config_parser_test.cc
	g++ $(GCOVFLAGS) $(TESTFLAGS) server_config_parser_test.cc $(SRC) ${GTEST_DIR}/src/gtest_main.cc libgtest.a -o $@ $(LDFLAGS)
	./$@

server_config_parser_gcov: server_config_parser_test
	gcov -r server_config_parser.cc > server_config_parser_gcov.txt

reverse_proxy_handler_test: test_setup reverse_proxy_handler.cc reverse_proxy_handler_test.cc
	g++ $(GCOVFLAGS) $(TESTFLAGS) reverse_proxy_handler_test.cc $(SRC) ${GTEST_DIR}/src/gtest_main.cc libgtest.a -o $@ $(LDFLAGS)
	./$@

reverse_proxy_handler_gcov: reverse_proxy_handler_test
	gcov -r reverse_proxy_handler.cc > reverse_proxy_handler_gcov.txt

docker:
	docker build -t httpserver.build .
	docker run httpserver.build > binary.tar
	rm -r -f deploy
	mkdir deploy
	cp $(DEPLOYFILES) deploy
	tar -xvf binary.tar -C deploy/
	cp Dockerfile.run deploy/Dockerfile
	docker build -t httpserver deploy

deploy:
	./deploy.sh

