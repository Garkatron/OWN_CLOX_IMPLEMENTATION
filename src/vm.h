#ifndef clox_vm_h
#define clox_vm_h
#define STACK_MAX 256

#include "chunk.h"
#include "value.h"

typedef struct
{
    Chunk *chunk;
    uint8_t *ip;
    int stackCount;
    int stackCapacity;
    Value *stack; // LIFO PILE
    Value *stackTop;        // Points just past the last item
} VM;

typedef enum
{
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

void initVM();
void freeVM();
InterpretResult interpret(Chunk *chunk);
void push(Value value);
Value pop();
Value getCurrent();
void modifyCurrent(Value value);

#endif