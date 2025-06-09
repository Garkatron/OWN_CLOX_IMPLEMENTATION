#ifndef clox_debug_h
#define clox_debug_h

#include "chunk.h"

// Disassembles all instructions inside the given chunk.
void disassembleChunk(Chunk *chunk, const char *name);
// Disassembles a instruction given the chunk and the offset.
int disassembleInstruction(Chunk *chunk, int offset);

#endif