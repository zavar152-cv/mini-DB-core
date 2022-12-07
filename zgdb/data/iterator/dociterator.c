#include "dociterator.h"

documentIterator createDocIterator(zgdbFile* file, uint64_t order, uint64_t orderParent) {
    documentIterator iterator;
    iterator.pStack = createStack();
    iterator.stop = false;
    nodeEntry next = {.order = order, .orderParent = orderParent};
    iterator.next = next;
    return iterator;
}

void destroyDocIterator(documentIterator* iterator) {
    deleteStack(&iterator->pStack);
}

bool hasNextDoc(documentIterator* iterator) {
    return !iterator->stop;
}

document nextDoc(zgdbFile* file, documentIterator* iterator) {
    document toReturn;
    iterator->header = getDocumentHeader(file, iterator->next.order);
    printf("Visited document: %s, ", iterator->header.name);
    printf("parent: %llu (index: %llu)\n", iterator->next.orderParent, iterator->next.order);
    iterator->document.header = iterator->header;
    iterator->document.isRoot = isRootDocumentHeader(iterator->header);
    iterator->document.indexParent = iterator->next.orderParent;

    toReturn = iterator->document;

    if (iterator->document.header.indexBrother == 0 && iterator->document.header.indexSon == 0) {
        if (peek(iterator->pStack) == NULL) {
            iterator->stop = true;
        } else {
            nodeEntry* pEntry = pop(iterator->pStack);
            iterator->next.order = pEntry->order;
            iterator->next.orderParent = pEntry->orderParent;
        }
    } else if (iterator->document.header.indexBrother != 0 && iterator->document.header.indexSon != 0) {
        iterator->next.order = iterator->document.header.indexBrother;
        iterator->next.orderParent = iterator->document.indexParent;
        iterator->temp.order = iterator->document.header.indexSon;
        iterator->temp.orderParent = iterator->document.header.indexAttached;
        push(iterator->pStack, iterator->temp);
    } else if (iterator->document.header.indexBrother != 0 && iterator->document.header.indexSon == 0) {
        iterator->next.order = iterator->document.header.indexBrother;
        iterator->next.orderParent = iterator->document.indexParent;
    } else if (iterator->document.header.indexBrother == 0 && iterator->document.header.indexSon != 0) {
        iterator->next.order = iterator->document.header.indexSon;
        iterator->next.orderParent = iterator->document.header.indexAttached;
    }
    return toReturn;
}