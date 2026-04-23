CC=gcc
CFLAGS=-c -Wall -pthread

SOURCES=$(wildcard ./src/*.c)
HEADERS=$(wildcard ./src/*.h)

OBJS=$(SOURCES:.c=.o)
EXES=$(basename $(notdir $(SOURCES)))

.PHONY: all clean

all: $(EXES)

./src/%.o: ./src/%.c $(HEADERS)
	$(CC) $(CFLAGS) $< -o $@

compressor decompressor: %: ./src/%.o
	$(CC) $< -o $@ -lm

compressor_threads decompressor_threads: %: ./src/%.o
	$(CC) $< -o $@ -lm -pthread

compressor_fork decompressor_fork: %: ./src/%.o
	$(CC) $< -o $@ -lm

clean:
	rm -f $(OBJS) $(EXES)