#ifndef ZGDBPROJECT_TREESTACK_H
#define ZGDBPROJECT_TREESTACK_H

#define STACK_OVERFLOW  (-100)
#define STACK_UNDERFLOW (-101)
#define OUT_OF_MEMORY   (-102)

#define INIT_SIZE 20
#define MULTIPLIER 2

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct nodeEntry {
    uint64_t order;
    uint64_t orderParent;
    uint64_t depth;
} nodeEntry;

typedef struct treeStack {
    nodeEntry* data;
    size_t size;
    size_t top;
} treeStack;

treeStack* createStack();

void deleteStack(treeStack** stack);

void resize(treeStack* stack);

void push(treeStack* stack, nodeEntry value);

nodeEntry* pop(treeStack* stack);

nodeEntry* peek(const treeStack* stack);

void implode(treeStack* stack);

#endif
