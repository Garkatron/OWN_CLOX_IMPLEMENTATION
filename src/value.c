#include <stdio.h>
#include "memory.h"
#include "value.h"

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

    default:
        break;
    }
}