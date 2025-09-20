 
CXX = g++
CXXFLAGS = -Wall -g -std=c++17

TARGET = program

SRC = main.cpp complex.cpp
OBJ = $(SRC:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJ)

# cpp into  o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

#  clean make option
clean:
	rm -f $(OBJ) $(TARGET)
