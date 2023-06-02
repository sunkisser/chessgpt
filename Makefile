# Compiler
CC = gcc

# Compiler flags
CFLAGS = -fopenmp -Wall -Wextra -Wpedantic -O3

# Source files
SRCS = attacked.c board.c evaluate.c game.c main.c move.c perft.c search.c uci.c zobrist.c

# Object files
OBJS = $(SRCS:.c=.o)

# Executable
TARGET = chessgpt

# Default target
all: $(TARGET)

# Compile source files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Link object files to create the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

# Format source files using Clang-Format
format:
	@echo "Formatting source files..."
	@clang-format -i $(SRCS) --output-replacements-xml | grep "<replacement " | wc -l | xargs echo "Replaced "

# Clean object files and executable
clean:
	rm -f $(OBJS) $(TARGET) logfile.txt
