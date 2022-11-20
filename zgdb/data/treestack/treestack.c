#include "treestack.h"

treeStack* createStack() {
    treeStack* out = NULL;
    out = malloc(sizeof(treeStack));
    if (out == NULL) {
        exit(OUT_OF_MEMORY);
    }
    out->size = INIT_SIZE;
    out->data = malloc(out->size * sizeof(nodeEntry));
    if (out->data == NULL) {
        free(out);
        exit(OUT_OF_MEMORY);
    }
    out->top = 0;
    return out;
}

void deleteStack(treeStack** stack) {
    free((*stack)->data);
    free(*stack);
    *stack = NULL;
}

void resize(treeStack* stack) {
    stack->size *= MULTIPLIER;
    stack->data = realloc(stack->data, stack->size * sizeof(nodeEntry));
    if (stack->data == NULL) {
        exit(STACK_OVERFLOW);
    }
}

void push(treeStack* stack, nodeEntry value) {
    if (stack->top >= stack->size) {
        resize(stack);
    }
    stack->data[stack->top] = value;
    stack->top++;
}


nodeEntry* pop(treeStack* stack) {
    if (stack->top == 0) {
        return NULL;
        //exit(STACK_UNDERFLOW);
    }
    stack->top--;
    return &stack->data[stack->top];
}

nodeEntry* peek(const treeStack* stack) {
    if (stack->top <= 0) {
        //exit(STACK_UNDERFLOW);
        return NULL;
    }
    return &stack->data[stack->top - 1];
}

void implode(treeStack* stack) {
    stack->size = stack->top;
    stack->data = realloc(stack->data, stack->size * sizeof(nodeEntry));
}