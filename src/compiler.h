#ifndef clox_compiler_h
#define clox_compiler_h

#include "vm.h"
#include "object.h"

// Given source code, it compiles it by writing bytes into the chunk.
ObjFunction* compile(const char *source);

#endif