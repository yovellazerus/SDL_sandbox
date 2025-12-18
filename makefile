# Compiler
CC = gcc

# Project name
TARGET = .\src\app.exe

# Source files
SRC = .\src\main.c

# SDL3 paths
SDL_INC = C:/Users/yovel/Desktop/VScode/SDL_sandbox/SDL3-3.2.28/x86_64-w64-mingw32/include
SDL_LIB = C:/Users/yovel/Desktop/VScode/SDL_sandbox/SDL3-3.2.28/x86_64-w64-mingw32/lib

# Compiler flags
CFLAGS = -I$(SDL_INC)
LDFLAGS = -L$(SDL_LIB) -lSDL3

# Default target
all: $(TARGET)

# Compile and link
$(TARGET): $(SRC)
	$(CC) $(SRC) $(CFLAGS) $(LDFLAGS) -o $(TARGET)

# Clean build
clean:
	if exist $(TARGET) del /F /Q $(TARGET)


# Phony targets
.PHONY: all clean
