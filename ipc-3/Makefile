CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -pthread
LDFLAGS := -lrt

all: first second

first: first.cpp shared_array.h
	$(CXX) $(CXXFLAGS) -o $@ first.cpp $(LDFLAGS)

second: second.cpp shared_array.h
	$(CXX) $(CXXFLAGS) -o $@ second.cpp $(LDFLAGS)

clean:
	rm -f first second

.PHONY: all clean
