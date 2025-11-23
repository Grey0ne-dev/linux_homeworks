# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2 -fPIC -pthread

# Sources
LIB_SRC = paralel_scheduler.c
LIB_OBJ = $(LIB_SRC:.c=.o)
LIB_TARGET = libparalel_scheduler.so

MAIN_SRC = main.c
MAIN_TARGET = main

# Default target
all: $(LIB_TARGET) $(MAIN_TARGET)

# Build shared library
$(LIB_TARGET): $(LIB_OBJ)
	$(CC) -shared -o $@ $^

# Compile object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Build main program linking against the shared library
$(MAIN_TARGET): $(MAIN_SRC) $(LIB_TARGET)
	$(CC) $(MAIN_SRC) -L. -lparalel_scheduler -pthread -o $@

# Clean up
clean:
	rm -f $(LIB_OBJ) $(LIB_TARGET) $(MAIN_TARGET)

.PHONY: all clean

