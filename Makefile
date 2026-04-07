CC = gcc
CXX = g++

CFLAGS = -Wall -Wextra -g -Iinclude
CXXFLAGS = -Wall -Wextra -g -Iinclude -std=c++17

TARGET = scipio

C_SRCS = $(wildcard src/decoder/*.c)
CXX_SRCS = $(wildcard src/graph/*.cpp) src/main.cpp

C_OBJS = $(C_SRCS:.c=.o)
CXX_OBJS = $(CXX_SRCS:.cpp=.o)
OBJS = $(C_OBJS) $(CXX_OBJS)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean