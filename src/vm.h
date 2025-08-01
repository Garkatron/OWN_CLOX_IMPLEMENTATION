#ifndef clox_vm_h
#define clox_vm_h
#define STACK_MAX 256

#include "object.h"
#include "table.h"
#include "value.h"

#define FRAMES_MAX 64

// A CallFrame represents a single ongoing function call
typedef struct
{
    ObjClosure* closure;
    uint8_t* ip;
    Value* slots;
} CallFrame;

typedef struct
{
    bool replMode;
    CallFrame frames[FRAMES_MAX];
    int frameCount;
    int stackCount;
    int stackCapacity;
    Value *stack;    // LIFO PILE
    Value *stackTop; // Points just past the last item
    Table globals;
    Table strings;
    ObjUpvalue* openUpvalues;
    Obj *objects;    // Objects list
} VM;

typedef enum
{
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

extern VM vm;

void initVM();
void freeVM();
/*
Given source code, it compiles it into a chunk. If compilation succeeds, it runs the code; otherwise, it frees the chunk and reports a compilation error.
*/
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