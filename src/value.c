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

    case VAL_OBJ:
        printObject(value);
        break;

    case VAL_NIL:
        printf("NIL");
        break;

    case VAL_NUMBER:

        printf("%g", AS_NUMBER(value));
        break;

    default:
        printf("Unknown value type: %d", value.type);
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
    case VAL_OBJ:
    {
        ObjString *aString = AS_STRING(a);
        ObjString *bString = AS_STRING(b);
        return aString->length == bString->length &&
               memcmp(aString->chars, bString->chars,
                      aString->length) == 0;
    }

    default:
        return false; // Unreachable.
    }
}

bool valuesEqualPointers(Value *a, Value *b)
{
    if (a->type != b->type)
        return false;
    switch (a->type)
    {
    case VAL_BOOL:
        return a->as.boolean == b->as.boolean;
    case VAL_NIL:
        return true;

    case VAL_NUMBER:
        return a->as.number == b->as.number;

    // If both are strings and have the same length checks the characters.
    case VAL_OBJ:
    {
        ObjString *aString = ((ObjString *)a->as.obj);
        ObjString *bString = ((ObjString *)b->as.obj);
        return aString->length == bString->length &&
               memcmp(aString->chars, bString->chars,
                      aString->length) == 0;
    }

    default:
        return false; // Unreachable.
    }
}