# Simple Makefile for main.cpp

# Compiler and flags
CXX      = g++
CXXFLAGS = -Wall -Wextra -pedantic -std=c++17

# Target program name
TARGET = bigdouble

# List of object files
OBJS = main.o

# The default rule to build the program
all: $(TARGET)

# Link object files into the final executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Compile .cpp to .o
main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp

# Optional: remove compiled files
clean:
	rm -f $(TARGET) $(OBJS)
