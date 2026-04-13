CC=gcc
CFLAGS=-c -Wall
SOURCE=./src/compresser.c
OBJ=$(SOURCE:.c=.o)
EXE=compresser
HEADERS=./src/compress.h ./src/tree.h
all: $(SOURCE) $(EXE)

$(EXE): $(OBJ)
	$(CC) $(OBJ) -o $@ -lm 

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $< -o $@ -lm

clean:
	rm -rf $(OBJ) $(EXE)
