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
    Value *stack;    // LIFO PILE
    Value *stackTop; // Points just past the last item
} VM;

typedef enum
{
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

void initVM();
void freeVM();
// Interprets the source code.
InterpretResult interpret(const char *source);
// Push a value into the stack and increase the stackTop
void push(Value value);
// Push back the stackTop and returns the "deleted" value.
Value pop();
// Gets the current value from the slot.
Value getCurrent();
// Modifies the current slot with the given value.
void modifyCurrent(Value value);

#endif