# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++11 -Iinclude

# Source files
SRCS = src/main.cpp src/Server.cpp src/ThreadPool.cpp src/ClientHandler.cpp src/RateLimiter.cpp src/ReverseProxy.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Target executable
TARGET = server

# Config header file
CONFIG = include/config.hpp

# Build rule
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Compile source files into object files
%.o: %.cpp $(CONFIG)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
