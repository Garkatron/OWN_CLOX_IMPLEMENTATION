#ifndef clox_chunk_h
#define clox_chunk_h
#include "common.h"
#include "memory.h"
#include "value.h"

typedef enum
{
    OP_CONSTANT,
    OP_CONSTANT_LONG,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NEGATE,
    OP_RETURN,
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
void writeChunk(Chunk *chunk, uint8_t byte, int line);
void writeConstant(Chunk *chunk, Value value, int line);
int getLine(Chunk *chunk, int index);
int addConstant(Chunk *chunk, Value value);

#endif