#ifndef clox_value_h
#define clox_value_h

#include "common.h"

typedef enum
{
    VAL_BOOL,
    VAL_NIL,
    VAL_NUMBER
} ValueType;

// This typedef abstracts how Lox values are concretely represented in C
typedef struct
{
    ValueType type;
    union
    {
        bool boolean;
        double number;
    } as;

} Value;

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