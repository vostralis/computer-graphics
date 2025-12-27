CXX = g++
CXXFLAGS = -O2 -Wall -Wextra -pedantic
INCLUDES = -Iinclude
LDFLAGS = -lglfw -lGL -lX11 -lpthread -lXi -ldl

all: main

main:
	$(CXX) $(CXXFLAGS) $(INCLUDES) src/* -o $@ $(LDFLAGS)