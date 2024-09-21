CXX = g++
CXXFLAGS = -Wall -Wextra
TARGET = internal_server
SRC = internal_server.cpp
OBJ = $(SRC:.cpp=.o)
INCLUDES = config.hpp

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $@

%.o: %.cpp $(INCLUDES)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
