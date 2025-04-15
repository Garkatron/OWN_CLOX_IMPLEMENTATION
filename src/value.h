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

// Return true has the given type.
#define IS_BOOL(value) ((value).type == VAL_BOOL)
#define IS_NIL(value) ((value).type == VAL_NIL)
#define IS_NUMBER(value) ((value).type == VAL_NUMBER)

// Returns the corresponding raw C value.
#define AS_BOOL(value) ((Value).as.boolean)
#define AS_NUMBER(value) ((Value).as.number)

// Takes a C value of the appropiate type and produces a Value with the correct ype tag and contains the underlying value.
#define BOOL_VAL(value)                \
    (Value)                            \
    {                                  \
        VAL_BOOL, { .boolean = value } \
    }
#define NIL_VAL(value) \
    (Value) { VAL_NIL, {.number = 0} }
#define NUMBER_VAL(value)               \
    (Value)                             \
    {                                   \
        VAL_NUMBER, { .number = value } \
    }

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