#include <stdio.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <wchar.h>
#include <locale.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <pthread.h>
#include "tree_threads.h"

bool fileExists(char *filename);
bool hasHuffExtension(char *filename, int size);
int filenameSize(char *filename);
long compressFile(char *filename, int matrix[256][256], long oldFileBytes);
void removeHuffExtension(char *original, char *copy, int size);
bool newDirectory(char *filename, char directoryName[500]);
void decompress(char *compressedFile, char directoryName[500]);
unsigned char findChar(FILE *file, Node *current, int *counter, unsigned char *byte, unsigned long *bytesLeft);

typedef struct DecompressArgs {
    long offset;
    unsigned long bytesLeft;
    char *compressedFile;
    char *fullPath;
    char *directoryName;
    Node *current;
} DecompressArgs;

Node huffmanTree[512];
int rootIndex;

int main(int argc, char *argv[]) {
    struct timeval start, end;
    
    // Validation of arguments number
    if (argc < 2) {
        printf("Usage: %s <file.huff>\n", argv[0]);
        return 1;
    }
    
    char directoryName[500];
    removeHuffExtension(argv[1], directoryName, filenameSize(argv[1]));
    
    //Validate file and create directory 
    if (!newDirectory(argv[1], directoryName)) {
        return 1;
    }

    gettimeofday(&start, NULL);
    decompress(argv[1], directoryName);
    gettimeofday(&end, NULL);

    double time = ((double)end.tv_sec - (double)start.tv_sec) * (double)1000 + ((double)end.tv_usec - (double)start.tv_usec) / (double)1000;
    printf("%f ms\n", time);
    return 0;
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

unsigned char findChar(FILE *file, Node *current, int *counter, unsigned char *byte, unsigned long *bytesLeft) {
    int validate;
    //Every 8 iterations a new byte is read
    while (current->key == -1) {
        if (*counter % 8 == 0) {
            
            //if (*bytesLeft == 0) return current->key;
            validate = fread(byte, sizeof(unsigned char), 1, file);
            (*bytesLeft) -= 1;
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

void *decompressFile(void *argsPointer) {
    DecompressArgs *arg = (DecompressArgs *)argsPointer;
    long offset = arg->offset;
    unsigned long bytesLeft = arg->bytesLeft;
    char *compressedFile = arg->compressedFile;
    char *directoryName = arg->directoryName;
    Node *current = arg->current;

    unsigned char byte, printingByte;
    char fileName[500];
    char fullPath[1000];
    int counter = 0;

    FILE *fileReading = fopen(compressedFile,"rb");
    //Recover filename
    fseek(fileReading, offset, SEEK_SET);
    for (int i = 0; fread(&byte, sizeof(unsigned char), 1, fileReading) == 1; i++) {
        fileName[i] = byte;
        bytesLeft--;
        if (byte == '\0') break;
    }
    snprintf(fullPath, sizeof(fullPath), "%s/%s", directoryName, fileName);
    FILE *fileWriting = fopen(fullPath, "wb");

    while (bytesLeft > 0) {
        //Traverse the tree and returns char
        printingByte = findChar(fileReading, current, &counter, &byte, &bytesLeft);
        fwrite(&printingByte, sizeof(unsigned char), 1, fileWriting);
        current = arg->current;
        //bytesLeft--;
    }
    counter = 0;
    byte = 0;
    fclose(fileWriting);

    return NULL;
}

void decompress(char *compressedFile, char directoryName[500]) {
    unsigned long bytesLeft;
    long frequencies[256];    
    //char fileName[500];
    //char fullPath[1000];
    int filesCount;
    
    FILE *fileReading = fopen(compressedFile,"rb");
    
    //Load frequencies
    fread(frequencies, sizeof(long), 256, fileReading);
    fread(&filesCount, sizeof(int), 1, fileReading);
    DecompressArgs args[filesCount];
    pthread_t threads[filesCount];
    
    //Create tree
    initializeNodes(frequencies, huffmanTree);
    rootIndex = createTree(huffmanTree);
    Node *root = &huffmanTree[rootIndex];
    //Node *current = &huffmanTree[rootIndex];
    
    //Write each file
    for (int i = 0; fread(&bytesLeft, sizeof(unsigned long), 1, fileReading)==1; i++) {
        args[i].offset = ftell(fileReading);
        args[i].bytesLeft = bytesLeft;
        args[i].compressedFile = compressedFile;
        args[i].directoryName = directoryName;
        args[i].current = root;
        pthread_create(&threads[i], NULL, decompressFile, &args[i]);
        fseek(fileReading, bytesLeft, SEEK_CUR);
    }
    for (int i = 0; i < filesCount; i++) {
        pthread_join(threads[i], NULL);
    }
    fclose(fileReading);
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