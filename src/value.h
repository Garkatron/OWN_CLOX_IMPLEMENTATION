#ifndef clox_value_h
#define clox_value_h

#include "common.h"

// This typedef abstracts how Lox values are concretely represented in C
typedef double Value;

typedef struct
{
    int count;
    int capacity;
    Value *values;
} ValueArray;

void initValueArray(ValueArray *valueArray);
void freeValueArray(ValueArray *valueArray);
void writeValueArray(ValueArray *valueArray, Value value);
void printValue(Value value);

#endif