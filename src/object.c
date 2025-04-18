#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "value.h"
#include "vm.h"

// Avoids redundantly cast to a void*.
#define ALLOCATE_OBJ(type, objectType) \
    (type *)allocateObject(sizeof(type), objectType)

// Allocates an object of the given size on the heap. Also you could need pass an extra size for payload fieds needed by specific objects.
static Obj *allocateObject(size_t size, ObjType type)
{
    Obj *object = (Obj *)reallocate(NULL, 0, size);
    object->type = type;
    return object;
}

// Allocate a new array on the heap. (Big as the string)
ObjString *copyString(const char *chars, int length)
{
    char *heapChars = ALLOCATE(char, length + 1);
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0'; // Monolithic source string isn't terminated.
    return allocateString(heapChars, length);
}

void printObject(Value value)
{
    switch (OBJ_TYPE(value))
    {
    case OBJ_STRING:
        printf("%s", AS_CSTRING(value));
        break;

    default:
        break;
    }
}

/*
Create a new ObjStrng on the heap and then initializes its fields.
*/
static ObjString *allocateString(char *chars, int length)
{
    ObjString *string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
    string->length = length;
    string->chars = chars;
    return string;
}
