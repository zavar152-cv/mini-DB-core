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

void insertNode(freeIndexesList* list, node* newNode) {
    if (list->head == NULL) {
        list->head = newNode;
        list->tail = newNode;
        list->head->prev = NULL;
        return;
    }

    if (newNode->blockSize < list->head->blockSize || newNode->blockSize == 0) {
        newNode->prev = NULL;
        list->head->prev = newNode;
        newNode->next = list->head;
        list->head = newNode;
        return;
    }

    if (newNode->blockSize > list->tail->blockSize) {
        newNode->prev = list->tail;
        list->tail->next = newNode;
        list->tail = newNode;
        return;
    }

    node* temp = list->head->next;
    while ((temp->blockSize) < (newNode->blockSize))
        temp = temp->next;

    temp->prev->next = newNode;
    newNode->prev = temp->prev;
    temp->prev = newNode;
    newNode->next = temp;
}

void insertNewIndex(freeIndexesList* list, uint64_t indexOrder) {
    node* newNode = createNewNode(indexOrder, 0);
    insertNode(list, newNode);
    list->newIndexesCount++;
    list->indexesCount++;
}

void insertDeadIndex(freeIndexesList* list, uint64_t indexOrder, uint64_t blockSize) {
    node* newNode = createNewNode(indexOrder, blockSize);
    insertNode(list, newNode);
    list->indexesCount++;
}

node* findRelevantIndex(freeIndexesList* list, uint64_t reqSize) {
    return NULL;
}

void printList(freeIndexesList* list) {
    node* temp = list->head;
    while (temp != NULL) {
        printf("%lu ", temp->blockSize);
        temp = temp->next;
    }
}
