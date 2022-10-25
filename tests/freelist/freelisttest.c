#include "../../zgdb/index/list/freelist.h"

int main() {
    freeIndexesList* pFreeIndexesList = createIndexesList();
    insertNewIndex(pFreeIndexesList, 1);
    insertNewIndex(pFreeIndexesList, 4);
    insertDeadIndex(pFreeIndexesList, 5, 64);
    insertDeadIndex(pFreeIndexesList, 5, 32);
    insertNewIndex(pFreeIndexesList, 9);
    printList(pFreeIndexesList);
    return 0;
}