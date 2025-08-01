#ifndef clox_chunk_h
#define clox_chunk_h
#include "common.h"
#include "memory.h"
#include "value.h"

typedef enum
{
    OP_CLOSE_UPVALUE,
    OP_LOOP,
    OP_CALL,
    OP_CLOSURE,
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_POP,
    OP_GET_LOCAL,
    OP_SET_LOCAL,
    OP_GET_GLOBAL,
    OP_DEFINE_GLOBAL,
    OP_SET_GLOBAL,
    OP_SET_UPVALUE,
    OP_GET_UPVALUE,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_CONSTANT,
    OP_CONSTANT_LONG,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NEGATE,
    OP_NOT,
    OP_PRINT,
    OP_RETURN
} OpCode;

typedef struct
{
    int line;
    uint8_t count;
} LineInfo;
typedef struct
{
    int count;
    int capacity;
    uint8_t *code;
    int lineCount;
    int lineCapacity;
    LineInfo *lines;
    ValueArray constants;
} Chunk;

void initChunk(Chunk *chunk);
void freeChunk(Chunk *chunk);

// Writes a byte into the chunk.
void writeChunk(Chunk *chunk, uint8_t byte, int line);

// Writes a constant to the constant pool of the chunk.
void writeConstant(Chunk *chunk, Value value, int line);

// Gets the line of the given index.
int getLine(Chunk *chunk, int index);

// Adds the given value to the end of the chunk's constant table and return its index.
int addConstant(Chunk *chunk, Value value);

#endif