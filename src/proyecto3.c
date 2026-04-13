#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <wchar.h>
#include <locale.h>
#include <string.h>
#include <ctype.h>

typedef struct Node {
    int key;
    long value;
    struct Node *leftChild;
    struct Node *rightChild;
    struct Node *parent;
} Node;


struct pair {
    char *key;
    long value;
};

bool validateFile(char *filename);
int fileSize(char *filename);
void createNodes(char *filename, Node* leafs);
void calculateFrecuencies(char *filename, struct pair characters[256]);
void loadFrecuenciesToFile(char *filename, struct pair characters[256]);
void initializeCharacters(struct pair characters[256]);
void printFrequencyTable(struct pair characters[256]);
int calcTreeSize(Node leafs[]);
void createMap(Node *root, int number[], int index, int matrix[][]);
void writeHuffmanTreeToFile(Node *leafs, int leafsIndex, const char *filename);


int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "");

    

    if (argc != 2) {
        printf("Usage: %s <frequency_file>\n", argv[0]);
        return 1;
    }

    char *freqFilename = argv[1];

    
    Node leafs[512];
    if (validateFile(freqFilename)) {
        createNodes(freqFilename, leafs);
        for (int i = 0; i < 512; i++) {
    }
    } else {
            printf("Frequency file not found or invalid.\n");
            return 1;
    }

    //Creating the tree
    int minIndex1, minIndex2, treeSize, leafsIndex;
    long minFreq1, minFreq2;
    treeSize = calcTreeSize(leafs);
    leafsIndex = 255;

    //From 256 onwars the Nodes are the parents including the root
    for (int i = 0;i<256;i++) {
        //Obtain the index of the minimum value
        minIndex1 = minIndex(leafs);
        minFreq1 = leafs[minIndex1].value;

        treeSize--;
        if (treeSize==0) {
            break;
        }
        
        //Freq is set to -1 to be ignored
        leafs[minIndex1].value = -1;

        //Obtain the index of the other minimum value
        minIndex2 = minIndex(leafs);
        minFreq2 = leafs[minIndex2].value;

        //Freq is set to -1 to be ignored
        leafs[minIndex2].value = -1; 

        //Create a new node that will compete with others to search the min value
        leafsIndex++;
        leafs[leafsIndex].key = NULL;
        leafs[leafsIndex].value = minFreq1 + minFreq2;

        //Assign children to the parent
        leafs[leafsIndex].leftChild = &leafs[minIndex1];
        leafs[leafsIndex].rightChild = &leafs[minIndex2];

        //Asign parent to the children
        leafs[minIndex1].parent = &leafs[leafsIndex];
        leafs[minIndex2].parent = &leafs[leafsIndex];
    }

    //writeHuffmanTreeToFile(leafs, leafsIndex, "huffmanTree.h");

    
    int array[256];       //worst case is a 256 code
    int matrix[256][256]; //To read the compressed binary code for each byte
    createMap(&leafs[leafsIndex],array,-1, matrix);
    //Traverse the map and create .h files
    
    return 0;
}

void createMap(Node *root, int binNumber[], int index, int matrix[256][256]) {
    //Base case: node does not have children (is a leaf)
    if (root->leftChild == NULL) {
        int symbol = root->key;

        int bit;
        for (int i = 0; i <= index; i++) {
            bit = binNumber[i];
            matrix[symbol][i] = bit;
        }
        matrix[symbol][index+1] = 2;
        return;
    }
    index++;
    binNumber[index] = 0;
    createMap(root->rightChild,binNumber,index);
    binNumber[index] = 1;
    createMap(root->leftChild,binNumber,index);    
}

int calcTreeSize(Node leafs[]) {
    int size = 0;
    for (int i = 0; i < 256; i++) {
        if (leafs[i].value >= 0) {
            size++;
        }
    }
    return size;
}

int minIndex(Node leafs[]) {
    int minIndex = -1;
    for (int i = 0; i<512; i++) {
        if (leafs[i].value >= (long)0) {
            if (minIndex == -1 || leafs[i].value <= leafs[minIndex].value) {
                minIndex = i;
            }
        }
    }
    return minIndex;
}


void createNodes(char *filename, Node* leafs) {

    FILE *file = fopen(filename,"r");
    setlocale(LC_ALL,"");                           //To show unicode chars
    long frequency;                               //Freq obtained by file
    char text[100];                                 //Store the char
    
    for (int i = 0; i < 512; i++) {
        leafs[i].leftChild = NULL;
        leafs[i].rightChild = NULL;
        leafs[i].parent = NULL;
        if (i<256 && !feof(file)) {
            if (i == 32) { // ASCII del espacio
                fscanf(file, " %ld", &frequency);
                leafs[i].key = i;
                leafs[i].value = frequency;
            } else {
                fscanf(file, "%s %ld", text, &frequency);
                leafs[i].key = i;
                leafs[i].value = frequency;
            }
        } else {
            //These nodes will be the sum nodes to create the tree
            leafs[i].key = NULL;
            leafs[i].value = -1;
        }
	}
    fclose(file);
}

bool validateFile(char *filename) {
    FILE *file = fopen(filename, "r");
    if (file != NULL) {
        fclose(file);
        return true;
    }
    return false;
}

int fileSize(char *filename) {
    int i;
    for (i = 0; filename[i] != '\0'; i++) {}
    return i;
}

void writeHuffmanTreeToFile(Node *leafs, int leafsIndex, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error opening file for writing.\n");
        return;
    }

    // Define the Node structure
    fprintf(file, "typedef struct HuffmanNode {\n");
    fprintf(file, "    int key;\n");
    fprintf(file, "    int left;\n");
    fprintf(file, "    int right;\n");
    fprintf(file, "} HuffmanNode;\n\n");

    // Write the array size
    fprintf(file, "#define HUFFMAN_TREE_SIZE %d\n\n", leafsIndex + 1);

    // Write the Huffman tree as an array
    fprintf(file, "const HuffmanNode huffmanTree[HUFFMAN_TREE_SIZE] = {\n");
    for (int i = 0; i <= leafsIndex; i++) {
        int key = (leafs[i].leftChild == NULL && leafs[i].rightChild == NULL) ? leafs[i].key : -1;
        fprintf(file, "    {.key = %d, .left = %d, .right = %d}",
                key,
                leafs[i].leftChild ? (int)(leafs[i].leftChild - leafs) : -1,
                leafs[i].rightChild ? (int)(leafs[i].rightChild - leafs) : -1);
        if (i < leafsIndex) {
            fprintf(file, ",\n");
        } else {
            fprintf(file, "\n");
        }
    }
    fprintf(file, "};\n");

    fclose(file);
}
