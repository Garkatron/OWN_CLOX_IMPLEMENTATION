#ifndef clox_compiler_h
#define clox_compiler_h

#include "vm.h"

// Given source code, it compiles it by writing bytes into the chunk.
bool compile(const char *source, Chunk *Chunk);

#endif