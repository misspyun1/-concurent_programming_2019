# Compiler and Compile options.
CC = g++
CFLAGS = -Wall -g -O2

# Macros specifying path for compile.
SRCS := $(wildcard src/*.cpp)
OBJS := $(SRCS:.cpp=.o)
INC = ./include/
LIB = ./lib/

# Pre-Processor.
CFLAGS += -I$(INC) -L$(LIB) -lpthread


TARGET = run


all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(CFLAGS)



# Delete binary & object files
clean:
	$(RM) $(TARGET) $(OBJS)
