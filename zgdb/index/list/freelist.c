#include <stdlib.h>
#include "freelist.h"

freeIndexesList createIndexesList() {
    freeIndexesList list;
    list.head = NULL;
    list.tail = NULL;
    list.indexesCount = 0;
    list.newIndexesCount = 0;
    return list;
}

void destroyIndexesList(freeIndexesList* list) {
    node* temp;
    while (list->head != NULL) {
        temp = list->head;
        list->head = list->head->next;
        free(temp);
    }
}


node* createNewNode(uint64_t indexOrder, uint64_t blockSize) {
    node* newNode = (node*) malloc(sizeof(node));
    newNode->blockSize = blockSize;
    newNode->indexOrder = indexOrder;
    newNode->next = NULL;
    newNode->prev = NULL;
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

    if (newNode->blockSize >= list->tail->blockSize) {
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

relevantIndexMeta* findRelevantIndex(freeIndexesList* list, uint64_t reqSize) {
    if(list->indexesCount == 1 && list->newIndexesCount == 1) {
        relevantIndexMeta* result = malloc(sizeof(relevantIndexMeta));
        result->indexOrder = list->head->indexOrder;
        result->blockSize = list->head->blockSize;
        free(list->head);
        list->head = NULL;
        list->tail = NULL;
        list->newIndexesCount--;
        list->indexesCount--;
        return result;
    }
    if(list->indexesCount == 1 && list->newIndexesCount == 0) {
        if(list->tail->blockSize >= reqSize) {
            relevantIndexMeta* result = malloc(sizeof(relevantIndexMeta));
            result->indexOrder = list->tail->indexOrder;
            result->blockSize = list->tail->blockSize;
            free(list->tail);
            list->head = NULL;
            list->tail = NULL;
            list->indexesCount--;
            return result;
        } else {
            return NULL;
        }
    }
    if(list->tail == NULL || list->head == NULL)
        return NULL;
    if(list->tail->blockSize >= reqSize) {
        node* pr = list->tail->prev;
        list->tail->prev = NULL;
        pr->next = NULL;
        relevantIndexMeta* result = malloc(sizeof(relevantIndexMeta));
        result->indexOrder = list->tail->indexOrder;
        result->blockSize = list->tail->blockSize;
        free(list->tail);
        list->tail = pr;
        list->indexesCount--;
        return result;
    } else if(list->head->blockSize == 0){
        node* pr = list->head->next;
        list->head->next = NULL;
        pr->prev = NULL;
        relevantIndexMeta* result = malloc(sizeof(relevantIndexMeta));
        result->indexOrder = list->head->indexOrder;
        result->blockSize = list->head->blockSize;
        free(list->head);
        list->head = pr;
        list->newIndexesCount--;
        list->indexesCount--;
        return result;
    } else {
        return NULL;
    }
}

void printFreeIndexesList(freeIndexesList* list) {
    node* temp = list->head;
    while (temp != NULL) {
        printf("%lu: %lu\n", temp->indexOrder, temp->blockSize);
        temp = temp->next;
    }
}
