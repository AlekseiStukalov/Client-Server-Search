CXX = clang++
MODE ?= release

ifeq ($(MODE),debug)
	CXXFLAGS = -std=c++20 -pedantic-errors -g -v
	SUFFIX = _debug
else
	CXXFLAGS = -std=c++20 -pedantic-errors
	SUFFIX =
endif

COMMON_SRC = Common/*.cpp

SERVER_SRC = Server/src/*.cpp Server/src/input/*.cpp Server/src/search/*.cpp
SERVER_OUT = jbha_server$(SUFFIX)
SERVER_FLAGS = -framework CoreServices

CLIENT_SRC = Client/src/*.cpp Client/src/input/*.cpp
CLIENT_OUT = jbha_client$(SUFFIX)

.PHONY: all server client clean

all: server client

server:
	$(CXX) $(CXXFLAGS) $(SERVER_SRC) $(COMMON_SRC) -o $(SERVER_OUT) $(SERVER_FLAGS)

client:
	$(CXX) $(CXXFLAGS) $(CLIENT_SRC) $(COMMON_SRC) -o $(CLIENT_OUT)

clean:
	rm -f jbha_server jbha_client jbha_server_debug jbha_client_debug
