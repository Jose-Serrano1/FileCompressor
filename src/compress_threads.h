#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdbool.h>
#include <dirent.h>
#include <math.h>
#include <libgen.h>

#define BUFFER_SIZE 100000

struct CompressArgs {
    char *fullPath;
    char *fileName;
    FILE *writingFile;
    char (*matrix)[256];;
} compArgs;

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

void compressFile(char fullPath[600], FILE *writingFile, char **matrix, char *filename) {
    unsigned char byteReading, byteWriting = 0;
    unsigned char buffer[BUFFER_SIZE];
    int i = 0;
    struct stat st;
    long oldFileSize; //Size in bytes

    //printf("Compressing %s\n", filename);

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
            i++;

            if (matrix[byteReading][j]) {
                byteWriting = byteWriting | 1;
            }

            if (i%8 != 0) continue;

            buffer[(i/8 - 1) % BUFFER_SIZE] = byteWriting;
            if (i/8 % BUFFER_SIZE == 0) {
                fwrite(buffer, sizeof(unsigned char), BUFFER_SIZE, writingFile);
            }
            //fwrite(&byteWriting, sizeof(unsigned char), 1, writingFile);
        }
        byteReading = 0;
    }

    int bytesInBuffer = (i / 8) % BUFFER_SIZE;
    if (i % 8 != 0) {
        byteWriting = byteWriting << (8 - (i % 8));
        buffer[bytesInBuffer] = byteWriting;
        bytesInBuffer++;
    }
    if (bytesInBuffer > 0) {
        fwrite(buffer, sizeof(unsigned char), bytesInBuffer, writingFile);
    }

    fclose(readingFile);
}


void *compressFileThread(void *argsPointer) {
    struct CompressArgs *args = (struct CompressArgs*)argsPointer;
    char (*matrix)[256] = args->matrix;
    char *filename = args->fileName;
    char *fullPath = args->fullPath;
    
    unsigned char byteReading, byteWriting = 0;
    unsigned char buffer[BUFFER_SIZE];
    int i = 0;
    unsigned long compressedSize = 0;

    FILE *readingFile = fopen(fullPath, "rb");
    if (readingFile == NULL) {
        printf("There was an error reading: %s\n", fullPath);
        return NULL;
    }

    FILE *temp = fopen(filename, "wb");
    if (temp == NULL ) {
        printf("There was an error compressing: %s\n", filename);
        return NULL;
    }

    //Save position to write size in bytes later
    fseek(temp, sizeof(unsigned long), SEEK_CUR);

    fwrite(filename, sizeof(char), strlen(filename) + 1, temp);
    compressedSize += strlen(filename)+1;

    while (fread(&byteReading, sizeof(unsigned char), 1, readingFile) == 1) {
        for (int j = 0; matrix[byteReading][j] != 2; j++) {
            byteWriting = byteWriting << 1;
            i++;

            if (matrix[byteReading][j]) {
                byteWriting = byteWriting | 1;
            }

            if (i%8 != 0) continue;

            buffer[(i/8 - 1) % BUFFER_SIZE] = byteWriting;
            if (i/8 % BUFFER_SIZE == 0) {
                compressedSize += BUFFER_SIZE;
                fwrite(buffer, sizeof(unsigned char), BUFFER_SIZE, temp);
            }
        }
        byteReading = 0;
    }

    int bytesInBuffer = ((i / 8) % BUFFER_SIZE);
    if (i % 8 != 0) {
        byteWriting = byteWriting << (8 - (i % 8));
        buffer[bytesInBuffer] = byteWriting;
        bytesInBuffer++;
    }
    if (bytesInBuffer > 0) {
        compressedSize += bytesInBuffer;
        fwrite(buffer, sizeof(unsigned char), bytesInBuffer, temp);
    }

    //Writing size after calculating it
    fseek(temp, 0, SEEK_SET);
    fwrite(&compressedSize, sizeof(unsigned long), 1, temp);
    fseek(temp, 0, SEEK_END);

    fclose(temp);
    fclose(readingFile);

    return filename;
}


void appendFile(char *filename, FILE *writing) {
    FILE *reading = fopen(filename,"rb");
    unsigned char buffer[BUFFER_SIZE];
    size_t bytesRead;

    while ((bytesRead = fread(buffer, sizeof(unsigned char), BUFFER_SIZE, reading))) {
        fwrite(buffer, sizeof(unsigned char), bytesRead, writing);
    }
    fclose(reading);
}


void compressDirectory(char *directory, char matrix[256][256], long frequencies[256], int filesCount) {
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
    
    pthread_t threads[filesCount];
    struct CompressArgs args[filesCount];
    for (int i = 0; (entry=readdir(folder)); i++) {

        if (!isTextFile(entry->d_name)) {i--; continue;}
        snprintf(fullPath, sizeof(fullPath), "%s/%s", directory, entry->d_name);

        args[i].fileName = strdup(entry->d_name);
        args[i].fullPath = strdup(fullPath);
        args[i].matrix = matrix;
        pthread_create(&threads[i], NULL, compressFileThread, &args[i]);
    }

    writingFile = fopen(newFile, "ab");//Huff file
    if (writingFile == NULL) {
        printf("There was an error during compressiion\n"); return;
    }
    fwrite(frequencies, sizeof(long), 256, writingFile); //Necessary to decompress
    fwrite(&filesCount, sizeof(int), 1, writingFile);

    void *filename;
    for (int i = 0; i < filesCount; i++) {
        pthread_join(threads[i], &filename);
        appendFile(filename, writingFile);
        remove(filename);
    }
    for(int i = 0; i < filesCount; i++) {
        free(args[i].fileName);
        free(args[i].fullPath);
    }

    closedir(folder);
    fclose(writingFile);
    free(newFile);
}