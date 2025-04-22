#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include "vm.h"
#include "common.h"
#include "debug.h"
#include "memory.h"
#include "compiler.h"
#include "value.h"
#include <string.h>

VM vm;

static void resetStack()
{
    vm.stackTop = vm.stack;
    vm.stackCapacity = STACK_MAX;
    vm.stackCount = 0;
}

static void runtimeError(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    size_t instruction = vm.ip - vm.chunk->code - 1;
    int line = vm.chunk->lines[instruction].line;
    fprintf(stderr, "[line %d] in script\n", line);
    resetStack();
}

void initVM()
{
    vm.stackCapacity = STACK_MAX;
    vm.stackCount = 0;
    resetStack();
    vm.objects = NULL;
}
void freeVM()
{
    freeObjects();
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

// Access the value
static Value peek(int distance)
{
    return vm.stackTop[-1 - distance];
}

static bool isFalsey(Value value)
{
    return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

/*
1. Calculates the length of the result string base on the lengths of the operands.
2. Allocates a character array for the result and then copy the two halves in.
3. '\n' finish string.
*/
static void concatenate()
{
    ObjString *b = AS_STRING(pop());
    ObjString *a = AS_STRING(pop());
    int length = a->length + b->length;
    char *chars = ALLOCATE(char, length + 1);
    memcpy(chars, a->chars, a - length);
    memcpy(chars + a->length, b->chars, b->length);
    chars[length] = '\0';

    ObjString *result = takeString(chars, length);
    push(OBJ_VAL(result));
}

Value getCurrent()
{
    if (vm.stackTop == vm.stack)
    {
        fprintf(stderr, "Runtime error: Stack underflow.\n");
        exit(1);
    }
    return *(vm.stackTop - 1);
}

void modifyCurrent(Value value)
{
    Value *current = vm.stackTop - 1;
    *current = value;
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
#define BINARY_OP(valueType, op)                        \
    do                                                  \
    {                                                   \
        if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) \
        {                                               \
            runtimeError("Operands must be numbers.");  \
            return INTERPRET_RUNTIME_ERROR;             \
        }                                               \
        double b = AS_NUMBER(pop());                    \
        double a = AS_NUMBER(pop());                    \
        push(valueType(a op b));                        \
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
            if (!IS_NUMBER(peek(0)))
            {
                runtimeError("Operand must be a number.");
                return INTERPRET_RUNTIME_ERROR;
            }
            push(NUMBER_VAL(-AS_NUMBER(pop())));
            break;
        case OP_ADD:
            if (IS_STRING(peek(0)) && IS_STRING(peek(1)))
            {
                concatenate();
            }
            else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1)))
            {
                double b = AS_NUMBER(pop());
                double a = AS_NUMBER(pop());
                push(NUMBER_VAL(a + b));
            }
            else
            {
                runtimeError("Operands must be two numbers or two strings.");
                return INTERPRET_RUNTIME_ERROR;
            }
            break;
        case OP_SUBTRACT:
            BINARY_OP(NUMBER_VAL, -);
            break;
        case OP_MULTIPLY:
            BINARY_OP(NUMBER_VAL, *);
            break;
        case OP_DIVIDE:
            BINARY_OP(NUMBER_VAL, /);
            break;

        case OP_NOT:
            push(BOOL_VAL(isFalsey(pop())));
            break;

        case OP_TRUE:
            push(BOOL_VAL(true));

            break;

        case OP_NIL:
            push(NIL_VAL);

            break;
        case OP_FALSE:
            push(BOOL_VAL(false));

        case OP_EQUAL:
        {
            Value b = pop();
            Value a = pop();
            push(BOOL_VAL(valuesEqual(a, b)));
            break;
        }

        case OP_GREATER:
            BINARY_OP(BOOL_VAL, >);
            break;
        case OP_LESS:
            BINARY_OP(BOOL_VAL, <);
            break;

#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
        }
    }
}

InterpretResult interpret(const char *source)
{
    Chunk chunk;
    initChunk(&chunk);

    if (!compile(source, &chunk))
    {
        freeChunk(&chunk);
        return INTERPRET_COMPILE_ERROR;
    }
    vm.chunk = &chunk;
    vm.ip = vm.chunk->code;
    InterpretResult result = run();
    freeChunk(&chunk);
    return result;
}
