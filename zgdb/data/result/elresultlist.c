#include "elresultlist.h"

eLresultList createElResultList() {
    eLresultList list;
    list.head = NULL;
    list.tail = NULL;
    list.size = 0;
    return list;
}

eLresult* getNextElOf(eLresult* of) {
    return of->next;
}

eLresult* getPrevElOf(eLresult* of) {
    return of->prev;
}

void destroyElResultList(eLresultList* list) {
    eLresult* temp;
    while (list->head != NULL) {
        temp = list->head;
        list->head = list->head->next;
        free(temp);
    }
}

void insertElResult(eLresultList* list, document document, element el) {
    eLresult* newResult = (eLresult*) malloc(sizeof(eLresult));
    newResult->document = document;
    newResult->el = el;
    newResult->next = NULL;
    newResult->prev = NULL;
    list->size++;

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

void printElResultList(eLresultList* list) {
    eLresult* temp = list->head;
    while (temp != NULL) {
        printf("Doc: %s, el: %s\n", temp->document.header.name, temp->el.key);
        temp = temp->next;
    }
}