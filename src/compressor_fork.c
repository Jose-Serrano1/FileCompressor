#include <sys/stat.h>
#include <stdbool.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <dirent.h>
#include <wchar.h>
#include <ctype.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>

#include "tree_threads.h"
#include "compress_fork.h"


void loadFrequenciesToMemory(char *filename, long frequencies[256]);
void calculateFrequencies(char *filename, long frequencies[256]);
void saveFrequenciesToFile(char *filename, long frequencies[256]);
void initializeFrequencies(long frequencies[256]);
void printFrequencyTable(long frequencies[256]);
void printNumericArray(long *array, int size);
bool validateFile(char *filename);
int fileSize(char *filename);

int main(int argc, char *argv[]) {

    struct timeval start, end;

    int filesCount = 0;
    char fullPath[600];

    if (argc < 2) {
        printf("Usage: %s <directory>\n", argv[0]);
        return 1;
    }

    /////DIRECTORY READING/////
    DIR *folder = opendir(argv[1]);
    struct dirent *entry;
    if (folder == NULL) {
        perror("Invalid directory\n"); return(1);
    }

    /////FREQUENCIES/////
    long frequencies[256];

    initializeFrequencies(frequencies);

    while ((entry=readdir(folder))) {
        if (!isTextFile(entry->d_name)) continue;

        filesCount++;
        snprintf(fullPath, sizeof(fullPath), "%s/%s", argv[1], entry->d_name);
        calculateFrequencies(fullPath, frequencies);
    }
    closedir(folder);
    
    /////TREE CREATION/////
    initializeNodes(frequencies, NULL);
    int rootIndex = createTree(NULL); 
    
    /////MAP BYTES TO BINARY CODE/////
    int array[256];
    char matrix[256][256]; //To read the compressed binary code for each byte
    createMap(&leafs[rootIndex], array, -1, matrix);

    gettimeofday(&start, NULL);
    compressDirectory(argv[1], matrix, frequencies, filesCount);
    gettimeofday(&end, NULL);
    double time = ((double)end.tv_sec - (double)start.tv_sec) * (double)1000 + ((double)end.tv_usec - (double)start.tv_usec) / (double)1000;
    printf("%f ms\n", time);

    return 0;
}

void printNumericArray(long *array, int size) {
    for(int i = 0; i < size; i++) {
        printf("%c %li\n", i, array[i]);
    }
    printf("\n");
}

void initializeFrequencies(long frequencies[256]) {
    for (int i = 0; i < 256; i++) {
        frequencies[i] = 0;
    }
}

void calculateFrequencies(char *filename, long frequencies[256]) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        printf("There was an error reading: %s\n", filename);
        return;
    }
    
    unsigned char buf[65536];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), file)) > 0) {
        for (size_t k = 0; k < n; k++) frequencies[buf[k]]++;
    }

    fclose(file);
    
}

void saveFrequenciesToFile(char *filename, long frequencies[256]) {
    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        printf("There was an error writing to: %s\n", filename);
        return;
    }
    fwrite(frequencies, sizeof(long), 256, file);
    fclose(file);
}

void loadFrequenciesToMemory(char *filename, long frequencies[256]) {
    FILE *file = fopen(filename,"rb");
    if (file == NULL) {
        printf("There was an error reading: %s\n", filename);
        return;
    }
    fread(frequencies, sizeof(long), 256, file);
    fclose(file);
}

bool validateFile(char *filename) {
    FILE *file = fopen(filename, "r");
    if (file != NULL) {
        fclose(file);
        return true;
    }
    printf("Archivo %s no leído\n", filename);
    return false;
}

int fileSize(char *filename) {
    int i;
    for (i = 0; filename[i] != '\0'; i++) {}
    return i;
}

void printFrequencyTable(long frequencies[256]) {
    printf("+------+-------+----------+\n");
    printf("| Hex  | Char  | Frequency|\n");
    printf("+------+-------+----------+\n");

    for (int i = 0; i < 256; i++) {
        char symbol = (isprint(i)) ? i : ' ';
        printf("| 0x%02X | %-5c | %-8ld |\n", (unsigned char)i, symbol, frequencies[i]);
    }

    printf("+------+-------+----------+\n");
}
