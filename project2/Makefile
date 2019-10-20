# Compiler and Compile options.
CXX = g++
CXXFLAGS = -Wall -g -O2 -std=c++11

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
	$(CXX) -o $(TARGET) $(OBJS) $(CFLAGS)



# Delete binary & object files
clean:
	$(RM) $(TARGET) $(OBJS)
