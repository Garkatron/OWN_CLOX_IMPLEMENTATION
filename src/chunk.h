#ifndef clox_chunk_h
#define clox_chunk_h
#include "common.h"
#include "memory.h"
#include "value.h"

typedef enum
{
    OP_CONSTANT,
    OP_RETURN,
} OpCode;

typedef struct
{
    int line;
    uint8_t count;
} Line;
typedef struct
{
    int count;
    int capacity;
    uint8_t *code;
    int lineCount;
    int lineCapacity;
    Line *lines;
    ValueArray constants;
} Chunk;

void initChunk(Chunk *chunk);
void freeChunk(Chunk *chunk);
void writeChunk(Chunk *chunk, uint8_t byte, int line);
int getLine(Chunk *chunk, int index);
int addConstant(Chunk *chunk, Value value);

#endif