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

//TODO optimization for middle
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

//TODO idea about block capacity
uint64_t* findRelevantIndex(freeIndexesList* list, uint64_t reqSize) {
    if(list->tail == NULL || list->head == NULL)
        return NULL;
    if(list->tail->blockSize >= reqSize) {
        node* pr = list->tail->prev;
        list->tail->prev = NULL;
        pr->next = NULL;
        uint64_t* order = malloc(sizeof(uint64_t));
        *order = list->tail->indexOrder;
        free(list->tail);
        list->tail = pr;
        return order;
    } else if(list->head->blockSize == 0){
        node* pr = list->head->next;
        list->head->next = NULL;
        pr->prev = NULL;
        uint64_t* order = malloc(sizeof(uint64_t));
        *order = list->head->indexOrder;
        free(list->head);
        list->head = pr;
        return order;
    } else {
        return NULL;
    }
}

void printList(freeIndexesList* list) {
    node* temp = list->head;
    while (temp != NULL) {
        printf("%lu: %lu ", temp->indexOrder, temp->blockSize);
        temp = temp->next;
    }
}
