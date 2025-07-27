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

ObjClosure* newClosure(ObjFunction* function) {
    ObjClosure* closure = ALLOCATE_OBJ(ObjClosure, OBJ_CLOSURE);
    closure->function = function;
    return closure;
}

ObjFunction* newFunction() {
    ObjFunction* function = ALLOCATE_OBJ(ObjFunction, OBJ_FUNCTION);
    function->arity = 0;
    function->upvalueCount = 0;
    function->name = NULL;
    initChunk(&function->chunk);
    return function;
}

ObjNative* newNative(NativeFn function) {
    ObjNative* native = ALLOCATE_OBJ(ObjNative, OBJ_NATIVE);
    native->function = function;
    return native;
}

/*
It allocates an object of the given size on the heap. Note that the size is not just the size of Obj itself. The caller passes in the number of bytes so that there is room for the extra payload fields needed by the specific object type being created.

Then it initializes the Obj state—right now, that’s just the type tag. This function returns to allocateString(), which finishes initializing the ObjString fields. Voilà, we can compile and execute string literals.
*/
static ObjString *allocateString(char *chars, int length, bool ownsChars, uint32_t hash)
{
    // Step 1: Allocate memory for an ObjString object. 
    // The ALLOCATE_OBJ macro handles memory allocation for the object, ensuring it gets the right size.
    ObjString *string = ALLOCATE_OBJ(ObjString, OBJ_STRING);

    // Step 2: Initialize the fields of the ObjString.
    string->ownsChars = ownsChars; // Whether this object owns the character array.
    string->chars = chars;
    string->length = length; // Length of the string.
    string->hash = hash; // Precomputed hash for the string (used for fast lookup).

    
    // Step 3: Set the string in the global string table (for tracking or interning strings).
    Value key = OBJ_VAL(string); // The key is the string object itself.
    Value value = NIL_VAL; // No associated value for the string (just tracking it).
    tableSet(&vm.strings, key, value); // Add the string object to the string table.
    
    // Step 4: Return the allocated and initialized ObjString.
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

Value copyString(const char *chars, int length) {
    // Calculate the hash value of the string to facilitate lookup
    uint32_t hash = hashString(chars, length);

    // Look up if the string is already interned in the string table
    ObjString *interned = tableFindString(&vm.strings, chars, length, hash);
    
    // If the string is already interned, return the pointer to the existing object
    if (interned != NULL) return OBJ_VAL(interned); 

    // If not found, allocate memory for a new string on the heap
    char *heapChars = ALLOCATE(char, length + 1);
    
    // Copy the contents of the original string into the new memory location
    memcpy(heapChars, chars, length);
    
    // Ensure the new string is properly null-terminated ('\0')
    heapChars[length] = '\0';

    // Create a new ObjString object with the copied string and associated information
    ObjString *str = allocateString(heapChars, length, true, hash);
    
    // Return the newly interned string object
    return OBJ_VAL(str);
}

ObjUpvalue* newUpvalue(Value* slot) {
    ObjUpvalue* upvalue = ALLOCATE_OBJ(ObjUpvalue, OBJ_UPVALUE);
    upvalue->location = slot;
    return upvalue;
}

static void printFunction(ObjFunction* function) {
    if(function->name == NULL) {
        printf("<script>");
        return;
    }
    printf("<fn %s>", function->name->chars);
}

void printObject(Value value)
{
    switch (OBJ_TYPE(value))
    {
    case OBJ_STRING:
        printf("\"%s\"", AS_CSTRING(value));
        break;
    case OBJ_FUNCTION:
        printFunction(AS_FUNCTION(value));
        break;
    case OBJ_NATIVE:
        printf("<native fn>");
        break;
    case OBJ_CLOSURE:
        printFunction(AS_CLOSURE(value)->function);
        break;
    
    case OBJ_UPVALUE:
        printf("upvalue");
        break;

    default:
        printf("Unknown object type: %d", OBJ_TYPE(value));
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
