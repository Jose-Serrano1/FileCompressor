CC=gcc
CFLAGS=-c -Wall -pthread

# Automatically find all source and header files in src/
SOURCES=$(wildcard ./src/*.c)
HEADERS=$(wildcard ./src/*.h)

# Generate object files and executable names
OBJS=$(SOURCES:.c=.o)
EXES=$(basename $(notdir $(SOURCES)))

.PHONY: all clean

all: $(EXES)

# Compile all .c files to .o files
./src/%.o: ./src/%.c $(HEADERS)
	$(CC) $(CFLAGS) $< -o $@

# Link executables - general rule for most
compressor decompressor: %: ./src/%.o
	$(CC) $< -o $@ -lm

# Link compressor_threads with pthread flag
compressor_threads: ./src/compressor_threads.o
	$(CC) $< -o $@ -lm -pthread

clean:
	rm -f $(OBJS) $(EXES)