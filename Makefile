# Compiler Settings
CC = gcc
CXX = g++

# Compiler Flags
CFLAGS = -Wall -Wextra -g -Iinclude
CXXFLAGS = -Wall -Wextra -g -Iinclude -std=c++17

# Linker Flags (External Libraries)
LDFLAGS = -lcapstone

# Project Name
TARGET = scipio
PREFIX = /usr/local/bin

# Source and Object Files
C_SRCS = $(wildcard src/decoder/*.c)
CXX_SRCS = $(wildcard src/graph/*.cpp) src/main.cpp

C_OBJS = $(C_SRCS:.c=.o)
CXX_OBJS = $(CXX_SRCS:.cpp=.o)
OBJS = $(C_OBJS) $(CXX_OBJS)

# ---------------------------------------------------
# Build Rules
# ---------------------------------------------------

all: $(TARGET)

# The Linking Step (Combines all .o files and adds Capstone)
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

# C Compilation Step
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# C++ Compilation Step
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

# ---------------------------------------------------
# Installation Rules
# ---------------------------------------------------
install: $(TARGET)
	@echo "[*] Installing $(TARGET) to $(PREFIX)..."
	@ln -sf $(abspath $(TARGET)) $(PREFIX)/$(TARGET)
	@echo "[+] Install complete. You can now run '$(TARGET)' from anywhere."

uninstall:
	@echo "[*] Removing $(TARGET) from $(PREFIX)..."
	@rm -f $(PREFIX)/$(TARGET)
	@echo "[+] Uninstall complete."

.PHONY: all clean install uninstall