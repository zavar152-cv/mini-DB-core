#include "resultlist.h"

resultList createResultList() {
    resultList list;
    list.head = NULL;
    list.tail = NULL;
    list.size = 0;
    return list;
}

void destroyResultList(resultList* list) {
    result* temp;
    while (list->head != NULL) {
        temp = list->head;
        list->head = list->head->next;
        free(temp);
    }
}

void insertResult(resultList* list, document document) {
    result* newResult = (result*) malloc(sizeof(result));
    newResult->document = document;
    newResult->next = NULL;
    newResult->prev = NULL;

    if (list->head == NULL) {
        list->head = newResult;
        list->tail = newResult;
        list->head->prev = NULL;
        return;
    }

    newResult->prev = list->tail;
    list->tail->next = newResult;
    list->tail = newResult;
}

void printResultList(resultList* list) {
    result* temp = list->head;
    while (temp != NULL) {
        printf("%s\n", temp->document.header.name);
        temp = temp->next;
    }
}