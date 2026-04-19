#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <wchar.h>
#include <locale.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include "tree.h"

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
void removeHuffExtension(char *original, char *copy, int size);
bool newDirectory(char *filename, char directoryName[500]);
void decompress(char *compressedFile, char directoryName[500]);

unsigned char findChar(FILE *file, Node *current, int *counter, unsigned char *byte);

Node huffmanTree[512];
int rootIndex;

int main(int argc, char *argv[]) {
    
    // Validation of arguments number
    if (argc < 2) {
        printf("Usage: %s <file.huff>\n", argv[0]);
        return 1;
    }
    
    /*long frequencies[256];
    FILE *file = fopen(argv[1],"rb");
    if (file == NULL) {
        printf("There was an error reading: %s\n", argv[1]);
        return;
    }
    fread(frequencies, sizeof(long), 256, file);*/

    char directoryName[500];
    removeHuffExtension(argv[1], directoryName, filenameSize(argv[1]));
    
    //Validate file and create directory 
    if (!newDirectory(argv[1], directoryName)) {
        return 1;
    }

    decompress(argv[1], directoryName);



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

bool newDirectory(char *filename, char directoryName[500]) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        printf("Could not open %s\n", filename);
        return false;
    }
    fclose(file);
    
    char command[500];
    DIR *folder;
    folder = opendir(directoryName);

    if (folder == NULL) {
        sprintf(command, "mkdir %s",directoryName);
    } else {
        char temp[500];
        sprintf(temp, "prev%s",directoryName);
        sprintf(directoryName, "%s",temp);
        sprintf(command, "mkdir %s",directoryName);

    }
    system(command);

    closedir(folder);
    return true;
}

unsigned char findChar(FILE *file, Node *current, int *counter, unsigned char *byte) {
    int validate;
    //Every 8 iterations a new byte is read
    while (current->key == -1) {
        if (*counter % 8 == 0) {
            validate = fread(byte, sizeof(unsigned char), 1, file);
            if (!validate) {
                printf("An error occured reading");
                fclose(file);
                exit(1);
            }
        }
        //Extract the most significant bit
        if ((*byte & 128) >> 7) {
            current = current->leftChild;
        } else {
            current = current->rightChild;
        }
        *byte = *byte << 1; 
        *counter += 1;
    }
    return (unsigned char)current->key;
}

/*void printStats(long oldSize, long newSize) {
    printf("      |%-5li          | %-5i          |%-1f                   |\n",oldSize, newSize, 100-(float)newSize*100/oldSize);
}*/




void decompress(char *compressedFile, char directoryName[500]) {
    long frequencies[256];    
    long decompressedFileSize;
    char fileName[500];
    char fullPath[1000];
    unsigned char byte;
    int counter = 0;
    unsigned char printingByte;
    
    FILE *fileReading = fopen(compressedFile,"rb");
    
    //Load frequencies
    fread(frequencies, sizeof(long), 256, fileReading);

    //Create tree
    initializeNodes(huffmanTree, frequencies);
    rootIndex = createTree(huffmanTree); 
    Node *root = &huffmanTree[rootIndex];
    Node *current = &huffmanTree[rootIndex];
    
    //Write each file
    while (fread(&decompressedFileSize, sizeof(long), 1, fileReading)==1) {
        //Recover filename
        for (int i = 0; fread(&byte, sizeof(unsigned char), 1, fileReading) == 1; i++) {
            fileName[i] = byte;
            if (byte == '\0') break;
        }
        snprintf(fullPath, sizeof(fullPath), "%s/%s", directoryName, fileName);
        printf("Writing %s\n",fullPath);
        FILE *fileWriting = fopen(fullPath, "wb");

        while (decompressedFileSize > 0) {
            //Traverse the tree and returns char
            printingByte = findChar(fileReading, current, &counter, &byte);
            fwrite(&printingByte, sizeof(unsigned char), 1, fileWriting);
            current = root;
            decompressedFileSize--;
        }
        counter = 0;
        byte = 0;
        fclose(fileWriting);

    }
    fclose(fileReading);

    
    

    

    

    /*char diffCommand[1000];
    sprintf(diffCommand, "diff %s %s.prev", uncompressedFile, uncompressedFile);
    printf("Comparing files %s and %s.prev:\n", uncompressedFile, uncompressedFile);
    system(diffCommand);*/
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