#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdbool.h>
#include <dirent.h>
#include <math.h>
#include <libgen.h>

bool isTextFile(char *filename) {
    int i;
    for(i = 0; filename[i]!='\0';i++) {}

    return i>3 && 
        filename[i-1]=='t' && 
        filename[i-2]=='x' && 
        filename[i-3]=='t';
}

void huffExtension(char *original, char *copy) {
    int i;
    for (i = 0; original[i] != '\0'; i++) {
        copy[i] = original[i];
    }
    copy[i] = '.';
    copy[i+1] = 'h';
    copy[i+2] = 'u';
    copy[i+3] = 'f';
    copy[i+4] = 'f';
    copy[i+5] = '\0';
}

void compressFile(char fullPath[600], FILE *writingFile, int matrix[256][256], char *filename) {
    unsigned char byteReading, byteWriting = 0;
    int i = 1;
    struct stat st;
    long oldFileSize; //Size in bytes

    if (!isTextFile(fullPath)) return;

    stat(fullPath, &st);
    oldFileSize = st.st_size;
    FILE *readingFile = fopen(fullPath, "rb");
    if (readingFile == NULL) {
        printf("There was an error reading: %s\n", fullPath);
        return;
    }

    fwrite(&oldFileSize, sizeof(long), 1, writingFile);
    fwrite(filename, sizeof(char), strlen(filename) + 1, writingFile);

    while (fread(&byteReading, sizeof(unsigned char), 1, readingFile) == 1) {
        //printf("chaa %02x\n",byteReading);
        for (int j = 0; matrix[byteReading][j] != 2; j++) {
            byteWriting = byteWriting << 1;
            if (matrix[byteReading][j]) {
                byteWriting = byteWriting | 1;
            }
            if (i%8 == 0) {
                fwrite(&byteWriting, sizeof(unsigned char), 1, writingFile);
            }
            i++;
        }
        byteReading = 0;
    }
    i--;
    if (i % 8 != 0) {
        byteWriting = byteWriting << (8-(i%8));
        fwrite(&byteWriting, sizeof(unsigned char), 1, writingFile);
    }
    fclose(readingFile);
}

void compressDirectory(char *directory, int matrix[256][256], long frequencies[256]) {
    char fullPath[600];
    FILE *writingFile;
    DIR *folder;
    struct dirent *entry;
    
    folder = opendir(directory);
    if (folder == NULL) {
        perror("Invalid directory\n"); return;
    }

    char *newFile = (char *)malloc(500 * sizeof(char));
    
    huffExtension(basename(directory),newFile);
    remove(newFile);

    writingFile = fopen(newFile, "ab");
    if (writingFile == NULL) {
        printf("There was an error during compressiion\n"); return;
    }
    fwrite(frequencies, sizeof(long), 256, writingFile); //Necessary to decompress
    
    while ((entry=readdir(folder))) {
        printf("Compressing %s\n", entry->d_name);
        snprintf(fullPath, sizeof(fullPath), "%s/%s", directory, entry->d_name);
        compressFile(fullPath, writingFile, matrix, entry->d_name);
    }

    closedir(folder);
    fclose(writingFile);
    free(newFile);
}
