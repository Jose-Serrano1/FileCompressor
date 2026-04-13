#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <wchar.h>
#include <locale.h>
#include <string.h>
#include <ctype.h>
#include "matrixHuffman.h"
#include "huffmanTree.h"

struct pair
{
    char *key;
    long value;
};


bool fileExists(char *filename);
bool hasHuffExtension(char *filename, int size);
int filenameSize(char *filename);
void loadFrecuenciesToMemory(char *filename, struct pair *characters);
void calculateFrecuencies(char *filename, struct pair characters[256]);
void loadFrecuenciesToFile(char *filename, struct pair characters[256]);
void initializeCharacters(struct pair characters[256]);
long compressFile(char *filename, int matrix[256][256], long oldFileBytes);
long countChars(char *filename);
void huffExtension(char *original, char *copy);
unsigned char findChar(FILE *file, HuffmanNode *current, int *counter, unsigned char *byte);

int main(int argc, char *argv[]) {

    // Validation of arguments number
    if (argc < 2) {
        printf("Usage: %s [input_files...]\n", argv[0]);
        return 1;
    }

    // Validations of files (if exist)
    for (int i = 1; i < argc; i++) {
        if (!fileExists(argv[i])) {
            printf("'%s' File not found or invalid.\n", argv[i]);
            return 1;
        }
    }

    //Decompressing
    for (int i = 1; i < argc; i++) {
        char * fileName = (char *)malloc(500 * sizeof(char));
        fileName = argv[i];
        int fileSize = filenameSize(fileName);
        if (hasHuffExtension(fileName, fileSize)) {
            printf("--------------------------------------------------\n");
            decompress(fileName);
        }
    }

    



/*
    long oldFileBytes = 0;
    long oldFileTotal = 0;

    long newFileTotal = 0;
    long newFileBytes = 0;
    struct stat st;

    printf("      _______________________________________________________________\n");
    printf("      |Old File Size  |New File Size   |Compaction Percentage       |\n");
    printf("      _______________________________________________________________\n");
    for (int i = 1; i < argc; i++) {

        //Validate the size of the uncompressed file
        stat(argv[i], &st);
        oldFileBytes = st.st_size;
        if (oldFileBytes > 16777215) {
            printf("%s file is too large, maximum is 16MB", argv[i]);
            return 1;
        }

        //Compress and accumulate stats
        newFileBytes = compressFile(argv[i], matrix, oldFileBytes); //Returns bytes of new file
        
        newFileTotal += newFileBytes;
        oldFileTotal += oldFileBytes;
        
        printStats(oldFileBytes, newFileBytes);
    }
    printf("Total:|%-5li          | %-5i          |%-1f                   |\n",oldFileTotal, newFileTotal, 100-(float)newFileTotal*100/oldFileTotal);
    printf("      ______________________________________________________________\n");
    */return 0;
}

unsigned char findChar(FILE *file, HuffmanNode *current, int *counter, unsigned char *byte) {
    int validate;
    //Every 8 iterations a new byte is read
    while (current->key == -1) {
        if (*counter % 8 == 0) {
            validate = fread(byte, sizeof(unsigned char), 1, file);
            if (!validate) {
                printf("An error occured reading %c",file);
                fclose(file);
                return;
            }
        }
        //Extract the most significant bit
        if ((*byte & 152) >> 7) {
            current = &huffmanTree[current->left];
        } else {
            current = &huffmanTree[current->right];
        }
        *byte = *byte << 1; 
        *counter += 1;
    }
    return (unsigned char)current->key;
}

void printStats(long oldSize, long newSize) {
    printf("      |%-5li          | %-5i          |%-1f                   |\n",oldSize, newSize, 100-(float)newSize*100/oldSize);
}

long countChars(char *filename) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        printf("There was an error counting: %s\n", filename);
        return;
    }
    unsigned char byte;
    long count = 0;
    while (fread(&byte, sizeof(unsigned char), 1, file)) {
        count++;
    }

    fclose(file);
    return count;
}



void decompress(char *compressedFile) {

    //uncompressedFile contains the original's file name
    char uncompressedFile[500];
    char prevFile[504];
    
    removeHuffExtension(compressedFile, uncompressedFile, filenameSize(compressedFile));
    sprintf(prevFile, "%s.prev", uncompressedFile);

    //If the original file is in the directory, it is renamed
    if (fileExists(uncompressedFile)) {
        if (fileExists(prevFile)) {
            remove(prevFile);
            printf("Previous %s.prev file removed\n", uncompressedFile);
        }
        char command[1000];
        sprintf(command, "mv %s %s.prev", uncompressedFile, uncompressedFile);
        system(command);

    }

    // Get uncompressed file size
    FILE *fileReading = fopen(compressedFile,"rb");
    unsigned char byte;
    int bytesTotal = 0;
    for (int i = 0; i < 3; i++) {
        fread(&byte,sizeof(unsigned char),1,fileReading);
        bytesTotal = bytesTotal | (byte<<(8*(2-i)));
    }

    int bytesWritten = 0;
    HuffmanNode *current = &huffmanTree[510];
    HuffmanNode *root = &huffmanTree[510];
    
    int counter = 0;
    unsigned char printingByte;

    FILE *fileWriting = fopen(uncompressedFile, "wb");

    while (bytesTotal > 0) {
        //Traverse the tree and returns char
        printingByte = findChar(fileReading, current, &counter, &byte);
        fwrite(&printingByte, sizeof(unsigned char), 1, fileWriting);
        current = root;
        bytesTotal--;
    }
    fclose(fileWriting);
    fclose(fileReading);

    char diffCommand[1000];
    sprintf(diffCommand, "diff %s %s.prev", uncompressedFile, uncompressedFile);
    printf("Comparing files %s and %s.prev:\n", uncompressedFile, uncompressedFile);
    system(diffCommand);
}

void removeHuffExtension(char *original, char *copy, int size) {
    for (int i = 0; i < size; i++) {
        copy[i] = original[i];
    }
    copy[size-5] = '\0'; 
}


bool fileExists(char *filename) {
    FILE *file = fopen(filename, "r");
    if (file != NULL)
    {
        fclose(file);
        return true;
    }
    return false;
}

int filenameSize(char *filename)
{
    int i;
    for (i = 0; filename[i] != '\0'; i++)
    {
    }
    return i;
}


bool hasHuffExtension(char *filename, int size) {
    return size >= 4 && filename[size-1] == 'f' && filename[size-2] == 'f' && 
        filename[size-3] == 'u' && filename[size-4] == 'h';
}