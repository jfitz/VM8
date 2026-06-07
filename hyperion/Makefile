TARGET = i8080_tests

C_FLAGS = -g -Wall -Wextra -O2 -std=c99 -pedantic
CPP_FLAGS = -g -Wall -MMD -MP
LDFLAGS =

CPP_SOURCES := $(wildcard *.cpp)
CPP_OBJECTS := $(CPP_SOURCES:.cpp=.o)

C_SOURCES := $(wildcard *.c)
C_OBJECTS := $(C_SOURCES:.c=.o)

DEPS := $(CPP_SOURCES:.cpp=.d)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(CPP_OBJECTS) $(C_OBJECTS)
	$(CXX) $^ $(LDFLAGS) -o "$@"

%.o: %.c
	$(CC) $(C_FLAGS) -c "$<" -o "$@"

%.o: %.cpp
	$(CXX) $(CPP_FLAGS) -c "$<" -o "$@"

-include $(DEPS)

clean:
	rm $(TARGET) $(C_OBJECTS) $(CPP_OBJECTS) $(DEPS)
