CXX     =g++
CXXFLAGS=-Wall -Wextra -Werror -pedantic -std=gnu++0x
%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c $<

all: client server

client: client.o
	$(CXX) $^ -o $@

server: server.o
	$(CXX) $^ -o $@

clean:
	rm -f *.o client server
