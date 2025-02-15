CC = gcc
CXX = g++

C_SOURCES = $(wildcard *.c)
CPP_SOURCES = $(wildcard *.cpp)
OBJECTS = $(C_SOURCES:%.c=%.o) $(CPP_SOURCES:%.cpp=%.o)

TARGET = i8080_tests

C_FLAGS = -g -Wall -Wextra -O2 -std=c99 -pedantic
CPP_FLAGS = -g -Wall
LDFLAGS =

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) -o $@ $^

%.o: %.c
	$(CC) $(C_FLAGS) -c -o $@ $<

%.o: %.cpp
	$(CXX) $(CPP_FLAGS) -c -o $@ $<
	
clean:
	-rm $(bin) $(obj)
