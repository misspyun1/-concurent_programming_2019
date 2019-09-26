#Compiler and Compile options.
CC = gcc
CFLAGS = -Wall -g -O2

# Macros specifying path for compile.
SRCS := $(wildcard src/*.c)
OBJS := $(SRCS:.c=.o)
INC = ./include/
LIB = ./lib/

# Pre-Processor.
CFLAGS += -I$(INC) -L$(LIB) -lpthread -O3


TARGET = run


all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(CFLAGS)



# Delete binary & object files
clean:
	$(RM) $(TARGET) $(OBJS)

