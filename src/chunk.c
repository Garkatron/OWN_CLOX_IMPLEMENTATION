#include <stdlib.h>
#include "chunk.h"

void initChunk(Chunk *chunk)
{
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->lineCount = 0;
    chunk->lineCapacity = 0;
    chunk->code = NULL;
    chunk->lines = NULL;
    initValueArray(&chunk->constants);
}

void writeChunk(Chunk *chunk, uint8_t byte, int line)
{
    if (chunk->capacity < chunk->count + 1)
    {
        int oldCapacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(oldCapacity);
        chunk->code = GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
        // chunk->lines = GROW_ARRAY(int, chunk->lines, oldCapacity, chunk->capacity);
    }
    chunk->code[chunk->count] = byte;

    // If the next byte it's in the same line of the previous, it increases the count of the previous one
    // If not, it adds a new line counter to the lines array.
    if (chunk->count != 0 && chunk->lines[chunk->lineCount - 1].line == line)
    {
        chunk->lines[chunk->lineCount - 1].count++;
    }
    else
    {
        if (chunk->lineCapacity < chunk->lineCount + 1)
        {
            int oldCapacity = chunk->lineCapacity;
            chunk->lineCapacity = GROW_CAPACITY(oldCapacity);
            chunk->lines = GROW_ARRAY(LineInfo, chunk->lines, oldCapacity, chunk->lineCapacity);
        }

        LineInfo l = {line, 1};
        chunk->lines[chunk->lineCount] = l;
    }
    chunk->count++;
}

void writeConstant(Chunk *chunk, Value value, int line) {
    int constIndex = addConstant(chunk, value);
    if (constIndex < 256) {
        writeChunk(chunk, OP_CONSTANT, line);
        writeChunk(chunk, constIndex, line);
    } else {
        writeChunk(chunk, OP_CONSTANT_LONG, line);
        writeChunk(chunk, (constIndex >> 16) & 0XFF, line);
        writeChunk(chunk, (constIndex >> 8) & 0XFF, line);
        writeChunk(chunk, constIndex & 0XFF, line);
    }
}

int addConstant(Chunk *chunk, Value value)
{
    writeValueArray(&chunk->constants, value);
    return chunk->constants.count - 1;
}
int getLine(Chunk *chunk, int index)
{
    if (index < 0 || index >= chunk->count)
        return -1;

    int counter = 0;
    for (int i = 0; i < chunk->lineCount; i++)
    {
        counter += chunk->lines[i].count;
        if (counter >= index)
        {
            return chunk->lines[i].line;
        }
    }
    return -1;
}

void freeChunk(Chunk *chunk)
{
    FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
    FREE_ARRAY(int, chunk->lines, chunk->capacity);
    freeValueArray(&chunk->constants);
    initChunk(chunk);
}
