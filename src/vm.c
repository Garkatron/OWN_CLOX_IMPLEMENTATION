#include <stdio.h>
#include "vm.h"
#include "common.h"
#include "debug.h"

VM vm;

void initVM()
{
    resetStack();
}
void freeVM()
{
}

void push(Value value)
{
    *vm.stackTop = value;
    vm.stackTop++;
}

Value pop()
{
    vm.stackTop--;
    return *vm.stackTop;
}

InterpretResult interpret(Chunk *chunk)
{
    vm.chunk = chunk;
    vm.ip = vm.chunk->code;
}

// The beating hearth of the VM...
// The most performance cost stuff occurs here.
// If you want to learn some of these techniques, look up “direct threaded code”, “jump table”, and “computed goto”.

static InterpretResult run()
{
#define READ_BYTE() (*vm.ip++)                                    // Next instruction
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()]) // Next byte from bytecode
    for (;;)
    {

#ifdef DEBUG_TRACE_EXECUTION
        disassembleInstruction(vm.chunk,
                               (int)(vm.ip - vm.chunk->code));
#endif

        uint8_t instruction;
        switch (instruction = READ_BYTE())
        {
        case OP_RETURN:
        {
            return INTERPRET_OK;
        }
        case OP_CONSTANT:
        {
            Value constant = READ_CONSTANT();
            printValue(constant);
            printf("\n");
            break;
        }
        case OP_CONSTANT_LONG:
        {
            Value constant = READ_CONSTANT();
            printValue(constant);
            printf("\n");
            break;
        }

#undef READ_BYTE
#undef READ_CONSTANT
        }
    }
}

static void resetStack()
{
    vm.stackTop = vm.stack;
}