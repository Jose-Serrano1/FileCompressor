#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct Node {
    int key;
    long value;
    struct Node *leftChild;
    struct Node *rightChild;
    struct Node *parent;
} Node;

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

int calcTreeSize(Node leafs[]) {
    int size = 0;
    for (int i = 0; i < 256; i++) {
        if (leafs[i].value >= 0) {
            size++;
        }
    }
    return size;
}

//Creating the tree
int createTree(Node *leafs) {
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
        leafs[leafsIndex].key = -1;
        leafs[leafsIndex].value = minFreq1 + minFreq2;

        //Assign children to the parent
        leafs[leafsIndex].leftChild = &leafs[minIndex1];
        leafs[leafsIndex].rightChild = &leafs[minIndex2];

        //Asign parent to the children
        leafs[minIndex1].parent = &leafs[leafsIndex];
        leafs[minIndex2].parent = &leafs[leafsIndex];
    }
    return leafsIndex;
}

//Map (matrix) created to get the binary code of each byte
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
    createMap(root->rightChild,binNumber,index, matrix);
    binNumber[index] = 1;
    createMap(root->leftChild,binNumber,index, matrix);
}

//Initialize nodes
void initializeNodes(Node* leafs, long frequencies[256]) {

    for (int i = 0; i < 512; i++) {
        leafs[i].leftChild = NULL;
        leafs[i].rightChild = NULL;
        leafs[i].parent = NULL;
        if (i<256) {
            leafs[i].key = i;
            leafs[i].value = frequencies[i];
        }
        else {
            leafs[i].key = -1;
            leafs[i].value = -1;
        }
    }
    
    /*FILE *file = fopen(filename,"r");
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
	}*/
}