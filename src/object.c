#include <stdio.h>
#include <string.h>

#include "object.h"
#include "value.h"
#include "vm.h"
#include "object.h"
#include "table.h"

// Avoids redundantly cast to a void*.
#define ALLOCATE_OBJ(type, objectType) \
    (type *)allocateObject(sizeof(type), objectType)

// Allocates an object of the given size on the heap. Also you could need pass an extra size for payload fieds needed by specific objects.
static Obj *allocateObject(size_t size, ObjType type)
{
    Obj *object = (Obj *)reallocate(NULL, 0, size);
    object->type = type;
    object->next = vm.objects;
    vm.objects = object;
    return object;
}

// Create a new ObjStrng on the heap and then initializes its fields.
static ObjString *allocateString(char *chars, int length, bool ownsChars, uint32_t hash)
{
    // size_t totalSize = sizeof() + length + 1;
    // OLD: ObjString *string = (ObjString *)malloc(totalSize); // Allocating memory
    ObjString *string = ALLOCATE_OBJ(ObjString, OBJ_STRING); // Allocating memory
    string->ownsChars = ownsChars;
    string->length = length;
    string->hash = hash;
    memcpy(string->chars, chars, length); // old: string->chars = chars;
    string->chars[length] = '\0';
    Value key = OBJ_VAL(string);
    Value value = NIL_VAL;
    tableSet(&vm.strings, key, value);
    return string;
}

// FNV-1a
static uint32_t hashString(const char *key, int length)
{
    uint32_t hash = 216613621u;
    for (int i = 0; i < length; i++)
    {
        hash ^= (uint8_t)key[i];
        hash *= 16777619;
    }
    return hash;
}

// Allocate a new array on the heap. (Big as the string)
Value copyString(const char *chars, int length) {
    uint32_t hash = hashString(chars, length);
    ObjString *interned = tableFindString(&vm.strings, chars, length, hash);
    if (interned != NULL) return OBJ_VAL(interned);

    char *heapChars = ALLOCATE(char, length + 1);
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';

    ObjString *str = allocateString(heapChars, length, true, hash);
    return OBJ_VAL(str);
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

ObjString *constString(const char *chars, int length)
{
    ObjString *str = ALLOCATE_OBJ(ObjString, OBJ_STRING);
    str->length = length;
    str->ownsChars = false;
    str->chars = (char *)chars;
    return str;
}

// Takes ownerships
ObjString *takeString(char *chars, int length) {
    uint32_t hash = hashString(chars, length);
    ObjString *interned = tableFindString(&vm.strings, chars, length, hash);
    if (interned != NULL) {
        FREE_ARRAY(char, chars, length + 1);
        return interned;
    }
    return allocateString(chars, length, true, hash);
}
