#include <stdlib.h>
#include "memory.h"
#include "vm.h"

void *reallocate(void *pointer, size_t oldSize, size_t newSize)
{
    if (newSize == 0)
    {
        free(pointer);
        return NULL;
    }

    void *result = realloc(pointer, newSize);
    if (result == NULL)
        exit(1);
    return result;
}

/*
This is a CS 101 textbook implementation of walking a linked list and freeing its nodes.
Frees objects specific memory
*/
static void freeObject(Obj *object)
{
    switch (object->type)
    {
    case OBJ_STRING:
    {
        ObjString *string = (ObjString *)object;

        if (string->ownsChars)
        {
            FREE_ARRAY(char, string->chars, string->length + 1);
        }
        else
        {
            FREE(ObjString, object);
        }
    }

    break;

    default:
        break;
    }
}

void freeObjects()
{
    Obj *object = vm.objects;
    while (object != NULL)
    {
        Obj *next = object->next;
        freeObject(object);
        object = next;
    }
}