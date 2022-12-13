#include "zgdbfilter.h"

path createPath(size_t n) {
    path p;
    p.size = n;
    p.capacity = 0;
    p.steps = (step*) malloc(n * sizeof(step));
    return p;
}

void addStep(path* p, step newStep) {
    if(p->capacity != p->size) {
        p->steps[p->capacity] = newStep;
        p->capacity++;
    }
}

void destroyPath(path* p) {
    for (size_t i = 0; i < p->size; ++i) {
        predicate* pPredicate = p->steps[i].pred;
        while (pPredicate != NULL) {
            predicate* nextPredicate = pPredicate->nextPredicate;
            free(pPredicate);
            pPredicate = nextPredicate;
        }
    }
    free(p->steps);
}