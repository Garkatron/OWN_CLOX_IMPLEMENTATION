#include <stdio.h>
#include "memory.h"
#include "value.h"
#include "object.h"
#include <string.h>

void writeValueArray(ValueArray *array, Value value)
{
    if (array->capacity < array->count + 1)
    {
        int oldCapacity = array->capacity;
        array->capacity = GROW_CAPACITY(oldCapacity);
        array->values = GROW_ARRAY(Value, array->values, oldCapacity, array->capacity);
    }
    array->values[array->count] = value;
    array->count++;
}
void initValueArray(ValueArray *array)
{
    array->count = 0;
    array->capacity = 0;
    array->values = NULL;
}
void freeValueArray(ValueArray *array)
{
    FREE_ARRAY(uint8_t, array->values, array->capacity);
    initValueArray(array);
}

void printValue(Value value)
{
    switch (value.type)
    {
    case VAL_BOOL:
        printf(AS_BOOL(value) ? "TRUE" : "FALSE");
        break;

    case VAL_NIL:
        printf("NIL");
        break;

    case VAL_NUMBER:
        printf("%g", AS_NUMBER(value));
        break;

    case VAL_OBJ:
        printObject(value);
        break;

    default:
        break;
    }
}

/*
First, we check the types. If the Values have different types, they are definitely not equal. Otherwise, we unwrap the two Values and compare them directly.
*/
bool valuesEqual(Value a, Value b)
{
    if (a.type != b.type)
        return false;
    switch (a.type)
    {
    case VAL_BOOL:
        return AS_BOOL(a) == AS_BOOL(b);
    case VAL_NIL:
        return true;

    case VAL_NUMBER:
        return AS_NUMBER(a) == AS_NUMBER(b);

    // If both are strings and have the same length checks the characters.
    case VAL_OBJ: return AS_OBJ(a) == AS_OBJ(b);

    default:
        return false; // Unreachable.
    }
}