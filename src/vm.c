#include <stdio.h>
#include "vm.h"
#include "common.h"
#include "debug.h"
#include "memory.h"

VM vm;

// The beating hearth of the VM...
// The most performance cost stuff occurs here.
// If you want to learn some of these techniques, look up “direct threaded code”, “jump table”, and “computed goto”.

static InterpretResult run()
{
#define READ_BYTE() (*vm.ip++)                                    // Next instruction
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()]) // Next byte from bytecode
    for (;;)
    {
#define BINARY_OP(op)     \
    do                    \
    {                     \
        double b = pop(); \
        double a = pop(); \
        push(a op b);     \
    } while (false)

#ifdef DEBUG_TRACE_EXECUTION
        printf("                                          ");
        for (Value *slot = vm.stack; slot < vm.stackTop; slot++)
        {
            printf("[ ");
            printValue(*slot);
            printf(" ]");
        }
        printf("\n");
        disassembleInstruction(vm.chunk,
                               (int)(vm.ip - vm.chunk->code));
#endif

        uint8_t instruction;
        switch (instruction = READ_BYTE())
        {
        case OP_RETURN:
        {
            printValue(pop());
            printf("\n");
            return INTERPRET_OK;
        }
        case OP_CONSTANT:
        {
            Value constant = READ_CONSTANT();
            push(constant);
            break;
        }
        case OP_CONSTANT_LONG:
        {
            uint8_t byte1 = READ_BYTE();
            uint8_t byte2 = READ_BYTE();
            uint8_t byte3 = READ_BYTE();
            int index = (byte1 << 16) | (byte2 << 8) | byte3;
            Value constant = vm.chunk->constants.values[index];
            push(constant);
            break;
        }

        case OP_NEGATE:
            modifyCurrent(-getCurrent());
            break;
        case OP_ADD:
            BINARY_OP(+);
            break;
        case OP_SUBTRACT:
            BINARY_OP(-);
            break;
        case OP_MULTIPLY:
            BINARY_OP(*);
            break;
        case OP_DIVIDE:
            BINARY_OP(/);
            break;

#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
        }
    }
}

static void resetStack()
{
    vm.stackTop = vm.stack;
    vm.stackCapacity = STACK_MAX;
    vm.stackCount = 0;
}

void initVM()
{
    vm.stackCapacity = STACK_MAX;
    vm.stackCount = 0;
    resetStack();
}
void freeVM()
{
    free(vm.stack);  // Free mem
    vm.stack = NULL; // Avoid old info (Safe)
    vm.stackCapacity = 0;
    vm.stackCount = 0;
}

void push(Value value)
{
    vm.stackCount++;
    if (vm.stackCount > vm.stackCapacity)
    {
        int oldCount = vm.stackCapacity;
        vm.stackCapacity = GROW_CAPACITY(vm.stackCapacity);
        vm.stack = GROW_ARRAY(Value, &vm.stack, oldCount, vm.stackCount);
    }
    *vm.stackTop = value;
    vm.stackTop++;
}

Value pop()
{
    if (vm.stackTop == vm.stack)
    {
        fprintf(stderr, "Runtime error: Stack underflow.\n");
        exit(1);
    }
    vm.stackTop--;
    return *vm.stackTop;
}

Value getCurrent()
{
    if (vm.stackTop == vm.stack)
    {
        fprintf(stderr, "Runtime error: Stack underflow.\n");
        exit(1);
    }
    return *(vm.stackTop-1);
}

void modifyCurrent(Value value) {
    Value *current = vm.stackTop-1;
    *current = value;
}

InterpretResult interpret(Chunk *chunk)
{
    printf("\n");
    printf("<-----------{ Interpreting }----------->\n");
    vm.chunk = chunk;
    vm.ip = vm.chunk->code;
    return run();
}
