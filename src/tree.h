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

Node leafs[512];

void minIndex(Node *treeArg, int *min1, int *min2) {
    Node *tree = treeArg ? treeArg : leafs;
    *min1 = -1;
    *min2 = -1;
    for (int i = 0; i < 512; i++) {
        if (tree[i].value < 0) continue; // nodo inactivo
        
        if (*min1 == -1 || tree[i].value < tree[*min1].value) {
            *min2 = *min1; // el anterior min1 pasa a ser min2
            *min1 = i;
        } else if (*min2 == -1 || tree[i].value < tree[*min2].value) {
            *min2 = i;
        }
    }
}

int calcTreeSize(Node *treeArg) {
    Node *tree = treeArg ? treeArg : leafs;
    int size = 0;
    for (int i = 0; i < 256; i++) {
        if (tree[i].value >= 0) {
            size++;
        }
    }
    return size;
}

//Creating the tree
int createTree(Node *treeArg) {
    Node *tree = treeArg ? treeArg : leafs;
    int minIndex1, minIndex2, treeSize, leafsIndex;
    long minFreq1, minFreq2;
    treeSize = calcTreeSize(tree);
    leafsIndex = 255;

    //From 256 onwars the Nodes are the parents including the root
    for (int i = 0;i<256;i++) {
        //Obtain the index of the minimum value
        minIndex(tree, &minIndex1, &minIndex2);
        treeSize--;
        if (treeSize==0) {
            break;
        }

        minFreq1 = tree[minIndex1].value;
        tree[minIndex1].value = -1;
        minFreq2 = tree[minIndex2].value;
        tree[minIndex2].value = -1; 

        //Create a new node that will compete with others to search the min value
        leafsIndex++;
        tree[leafsIndex].key = -1;
        tree[leafsIndex].value = minFreq1 + minFreq2;

        //Assign children to the parent
        tree[leafsIndex].leftChild = &tree[minIndex1];
        tree[leafsIndex].rightChild = &tree[minIndex2];

        //Asign parent to the children
        tree[minIndex1].parent = &tree[leafsIndex];
        tree[minIndex2].parent = &tree[leafsIndex];
    }
    return leafsIndex;
}

//Map (matrix) created to get the binary code of each byte
void createMap(Node *root, int binNumber[], int index, char matrix[256][256]) {
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
void initializeNodes(long frequencies[256], Node *treeArg) {
    Node *tree = treeArg ? treeArg : leafs;
    for (int i = 0; i < 512; i++) {
        tree[i].leftChild = NULL;
        tree[i].rightChild = NULL;
        tree[i].parent = NULL;
        if (i<256) {
            tree[i].key = i;
            tree[i].value = frequencies[i];
        }
        else {
            tree[i].key = -1;
            tree[i].value = -1;
        }
    }
}