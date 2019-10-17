INCLUDE := include
SRC := $(shell find src -type f -name "*.cpp")

GXX := g++
CXXFLAGS := -std=c++11
CPPFLAGS := -g -I $(INCLUDE)

DFLAGS := -g 

.PHONY: clean all setup

all: setup
	$(GXX) $(CXXFLAGS) $(CPPFLAGS) $(SRC) -o bin/program

debug: setup
	$(GXX) $(CXXFLAGS) $(CPPFLAGS) $(DFLAGS) $(SRC) -o bin/program

setup:
	mkdir -p bin

clean:
	$(RM) -r bin
