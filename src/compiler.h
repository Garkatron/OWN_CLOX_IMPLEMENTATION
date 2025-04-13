#ifndef clox_compiler_h
#define clox_compiler_h

#include "vm.h"

// Compiles the source code.
bool compile(const char *source, Chunk *Chunk);

#endif