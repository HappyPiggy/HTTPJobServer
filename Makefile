
#CXX = clang++
CXXFLAGS = -std=c++14  -Wall -Wextra -pthread -static-libstdc++ -static-libgcc
OPENSSL_DIR = /usr/local/opt/openssl
OPENSSL_SUPPORT = -DCPPHTTPLIB_OPENSSL_SUPPORT -I$(OPENSSL_DIR)/include -L$(OPENSSL_DIR)/lib -lssl -lcrypto
ZLIB_SUPPORT = -DCPPHTTPLIB_ZLIB_SUPPORT -lz

all: server docker

server : server.cc job.cpp httplib.h Makefile
	$(CXX) -o server $(CXXFLAGS) server.cc job.cpp $(OPENSSL_SUPPORT) $(ZLIB_SUPPORT)

pem:
	openssl genrsa 2048 > key.pem
	openssl req -new -key key.pem | openssl x509 -days 3650 -req -signkey key.pem > cert.pem
docker: server
	docker build -t HTTPJob:v1 .
	docker save -o install/HTTPJob.tar HTTPJob:v1

clean:
	rm server install/HTTPJob.tar 



