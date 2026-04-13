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

struct pair
{
    char *key;
    long value;
};

bool validateFile(char *filename);
int fileSize(char *filename);
void loadFrecuenciesToMemory(char *filename, struct pair *characters);
void calculateFrecuencies(char *filename, struct pair characters[256]);
void loadFrecuenciesToFile(char *filename, struct pair characters[256]);
void initializeCharacters(struct pair characters[256]);
long compressFile(char *filename, int matrix[256][256], long oldFileBytes);
long countChars(char *filename);
void huffExtension(char *original, char *copy);

int main(int argc, char *argv[]) {

    // Initialization of matrix
    initMatrix();

    // Validation of arguments number
    if (argc < 2) {
        printf("Usage: %s [input_files...]\n", argv[0]);
        return 1;
    }

    // Validations of files (if exist)
    for (int i = 1; i < argc; i++) {
        if (!validateFile(argv[i])) {
            printf("'%s' File not found or invalid.\n", argv[i]);
            return 0;
        }
    }


    long oldFileBytes = 0;
    long oldFileTotal = 0;

    long newFileTotal = 0;
    long newFileBytes = 0;
    struct stat st;

    printBorder();
    printTableHeader();
    printBorder();
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
        
        printStats(oldFileBytes, newFileBytes, argv[i]);
    }
    float average = 100-(float)newFileTotal*100/oldFileTotal;
    printBorder();
    printf("|%-20s|%-15li|%-15i|%-22.6f|\n", 
           "Total:", oldFileTotal, newFileTotal, average);
    printBorder();
    return 0;
}

// void printStats(long oldSize, long newSize, char *filename) {
//     if (oldSize > newSize) {
//         printf("|%-15s      |%-5li          | %-5i          |%-1f                 |\n", filename, oldSize, newSize, 100-(float)newSize*100/oldSize);
//     } else {
//         printf("|%-15s      |%-5li          | %-5i          |File's size increased|\n", filename,oldSize, newSize);
//     }
// }

void printTableHeader() {
    printf("|%-20s|%-15s|%-15s|%-22s|\n", "File Name", "Old File Size", "New File Size", "Compaction Percentage");
}

void printBorder() {
    printf("|--------------------+---------------+---------------+----------------------|\n");
}

void printStats(long oldSize, long newSize, char *filename) {
    float compressionRate = 100-(float)newSize*100/oldSize;
    printf("|%-20s|%-15li|%-15i|%-22.6f|\n", 
           filename, oldSize, newSize, compressionRate);
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

int binary(unsigned char number) {
    int numBits = sizeof(number) *8;
    for (int i = numBits - 1; i >= 0; i--) {
        int bit = (number >> i) & 1;
        printf("%d",bit);
    }
    printf("\n");
}

long compressFile(char **directory, int matrix[256][256], long oldFileBytes) {
    // SAVING BITS
    unsigned char byteReading = 0;
    unsigned char byteWriting = 0;
    double totalBits;

    //String with extension .huff
    char *newFile = (char *)malloc(500 * sizeof(char));
    huffExtension(filename,newFile);

    //Deleting old compressed file if exists
    remove(newFile);
    
    FILE *readingFile = fopen(filename, "rb");

    //Header with originals file size
    FILE *writingFile = fopen(newFile, "ab");
    if (writingFile == NULL || readingFile == NULL) {
        printf("There was an error reading: %s\n", filename);
        return 0;
    }
    unsigned char header[8];
    header[7] = oldFileBytes & 0xFF;
    header[6] = (oldFileBytes >> 8) & 0xFF;
    header[5] = (oldFileBytes >> 16) & 0xFF;
    header[4] = (oldFileBytes >> 24) & 0xFF;
    header[3] = (oldFileBytes >> 32) & 0xFF;
    header[2] = (oldFileBytes >> 40) & 0xFF;
    header[1] = (oldFileBytes >> 48) & 0xFF;
    header[0] = (oldFileBytes >> 56) & 0xFF;
    //3 bytes added to newFileSize
    totalBits += 24;
    fwrite(header, sizeof(unsigned char), 3, writingFile);
  
    
    int i = 1;
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
            totalBits++;
        }
        byteReading = 0;
    }
    //printf("\n\n\n");
    //printf("%02x\n",byteReading);
    i--;
    if (i % 8 != 0) {
      byteWriting = byteWriting << (8-(i%8));
    }
    fwrite(&byteWriting, sizeof(unsigned char), 1, writingFile);
    
    fclose(readingFile);
    fclose(writingFile);
      
    free(newFile);
    return ceil(totalBits/8);
/*
    bool *bits = (bool *)malloc(197772160 *sizeof(bool));
    int bitsCountTemp = 0;
    int newFileBytes = 0;
    int index = 0;
    while (newFileBytes < oldFileBytes) {
        
        for (int i = 0; matrix[byteReading][i] != 2; i++) {
            byteWriting = byteWriting << 1;
            if (matrix[byteReading][i]) {
                byteWriting = byteWriting | 1;
            }

            if ()
        }
        //------------------------------------------------------------//
        //-------------------------SAVING-----------------------------//
        //------------------------------------------------------------//
        
        while (bitsCountTemp < 197772160) {
            
            if (!feof(file)) {
                for (int i = 0; matrix[byteReading][i] != 2; i++) {
                    if (bitsCountTemp > 16777216) {
                        index = i;
                        bitsCountTemp--;
                        break;
                    }
                    bits[bitsCountTemp] = matrix[byteReading][i];
                    bitsCountTemp++;
                }
                newFileBytes++;
            }
            else {
                // This occurs at the end of the file
                oldFileBytes = newFileBytes;  // To end the while loop after writing
                break;
            }
        }
        fclose(file);

        //------------------------------------------------------------//
        //-------------------------WRITING----------------------------//
        //------------------------------------------------------------//
        //File handling
        file = fopen(newFile, "ab");
        if (file == NULL) {
            printf("There was an error reading: %s\n", filename);
            return 0;
        }
        for (int i = 1; i <= bitsCountTemp; i++) {
            byteWriting = byteWriting << 1;
            if (bits[i-1]) {
                byteWriting = byteWriting | 1;
            }
            //Every time a whole byte is written
            if (bitsCountTemp < 16) {
                printf("%i",bits[i-1]);
            }
            if ((i%8 == 0 && i != 0 || i == bitsCountTemp)) {
                //printf("aaaaaaaaaa%lf\n%i, bitsC:%i\n\n",totalBits/8,i,bitsCountTemp);
                if (bitsCountTemp <8192 && i == bitsCountTemp) {
                    byteWriting = byteWriting<<((i+8)%8);
                }
                f
                byteWriting = 0;
            }
            totalBits++;
        }
        bitsCountTemp = 0;
        fclose(file);
        
    }*/
    

        //Total of bytes of new file
    return ceil(totalBits/8);

}


bool validateFile(char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file != NULL)
    {
        fclose(file);
        return true;
    }
    return false;
}

int fileSize(char *filename)
{
    int i;
    for (i = 0; filename[i] != '\0'; i++)
    {
    }
    return i;
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
