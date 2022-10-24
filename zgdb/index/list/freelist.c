#include <stdlib.h>
#include "freelist.h"

freeIndexesList* createIndexesList() {
    freeIndexesList* list = (freeIndexesList*) malloc(sizeof(freeIndexesList));
    list->head = NULL;
    list->tail = NULL;
    list->indexesCount = 0;
    list->newIndexesCount = 0;
    return list;
}

//TODO 40 bits check
node* createNewNode(uint64_t indexOrder, uint64_t blockSize) {
    node* newNode = (node*) malloc(sizeof(node));
    newNode->blockSize = blockSize;
    newNode->indexOrder = indexOrder;
    newNode->next = NULL;
    return newNode;
}

void insertNewIndex(freeIndexesList* list, uint64_t indexOrder) {
    node* newNode = createNewNode(indexOrder, 0);
    newNode->next = list->head;
    list->head = newNode;
    //TODO tail???
}