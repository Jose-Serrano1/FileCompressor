CC=gcc
CFLAGS=-c -Wall
SOURCE=./src/compressor.c
SOURCE2=./src/decompresser.c
OBJ=$(SOURCE:.c=.o)
OBJ2=$(SOURCE2:.c=.o)
EXE=compressor
EXE2=decompressor
HEADERS=./src/compress.h ./src/tree.h

all: $(EXE) $(EXE2)

$(EXE): $(OBJ)
	$(CC) $(OBJ) -o $@ -lm

$(EXE2): $(OBJ2)
	$(CC) $(OBJ2) -o $@ -lm

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $< -o $@ -lm

clean:
	rm -rf $(OBJ) $(OBJ2) $(EXE) $(EXE2)