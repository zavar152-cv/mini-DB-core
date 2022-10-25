#include <time.h>
#include "../../zgdb/index/list/freelist.h"

int main() {
    freeIndexesList* pFreeIndexesList = createIndexesList();
    insertNewIndex(pFreeIndexesList, 1);
    insertNewIndex(pFreeIndexesList, 4);
    insertDeadIndex(pFreeIndexesList, 5, 64);

    struct timespec begin, end;

    for(uint64_t i = 0; i < 15; i++) {
        clock_gettime(CLOCK_REALTIME, &begin);
        insertDeadIndex(pFreeIndexesList, i, 20000 - i);
        clock_gettime(CLOCK_REALTIME, &end);
        long nanoseconds = end.tv_nsec - begin.tv_nsec;
        //printf("Ins: %llu, Time measured: %.10ld ns.\n", 20000 - i, nanoseconds);
        insertDeadIndex(pFreeIndexesList, 20000 + i, i);
    }

    insertNewIndex(pFreeIndexesList, 9);
    insertDeadIndex(pFreeIndexesList, 567, 40000);
    printList(pFreeIndexesList);

    uint64_t* pInt = findRelevantIndex(pFreeIndexesList, 56);
    printf("\n%lu\n", *pInt);
    printList(pFreeIndexesList);

    pInt = findRelevantIndex(pFreeIndexesList, 345);
    printf("\n%lu\n", *pInt);
    printList(pFreeIndexesList);

    pInt = findRelevantIndex(pFreeIndexesList, 234234);
    printf("\n%lu\n", *pInt);
    printList(pFreeIndexesList);
    return 0;
}