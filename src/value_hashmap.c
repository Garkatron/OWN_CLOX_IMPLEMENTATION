
#include "value_hashmap.h"
#include "value.h"
#include "object.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

bool vequal(const void *a_ptr, const void *b_ptr) 
{
    Value a = *(Value *)a_ptr;
    Value b = *(Value *)b_ptr;

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

    // Si ambos son cadenas y tienen la misma longitud, compara los caracteres
    case VAL_OBJ:
    {
        ObjString *aString = AS_STRING(a);
        ObjString *bString = AS_STRING(b);
        return aString->length == bString->length &&
               memcmp(aString->chars, bString->chars,
                      aString->length) == 0;
    }

    default:
        return false; // Inalcanzable
    }
}


void vfreeKey(void *key) {
    ObjString *objString;
    if (((Obj*)key)->type == OBJ_STRING) {
        objString = (ObjString*)key; 
        free(objString->chars); 
        free(objString);
    }
}

void vfreeValue(void *value) {
    ObjString *objString;
    if (((Obj*)value)->type == OBJ_STRING) {
        objString = (ObjString*)value; 
        free(objString->chars);
        free(objString);
    }
}

#include <stdint.h>
#include <string.h>

uint32_t vhashValue(const void *v) {
    Value *value = (Value *) v;
    uint32_t hash = 216613621u; // FNV-1a prime base

    switch (value->type) {
    case VAL_BOOL:
        // Para booleanos, simplemente usamos el valor como un hash.
        hash ^= (uint8_t)value->as.boolean;
        hash *= 16777619;
        break;

    case VAL_NIL:
        // Para NIL, podemos simplemente devolver un valor constante.
        hash ^= 0;  // No es necesario, pero se puede hacer explícito.
        hash *= 16777619;
        break;

    case VAL_NUMBER:
        // Para números (valores double), convertimos el número a un valor uint32_t para el hash.
        {
            uint32_t *numPtr = (uint32_t *)&value->as.number;
            hash ^= numPtr[0];  // Usamos el primer entero de la representación de un double
            hash *= 16777619;
        }
        break;

        case VAL_OBJ:
            switch (OBJ_TYPE(*value)) {
                case OBJ_STRING:
                    return AS_STRING(*value)->hash;

                default:
                    return 0;
                }

    default:
        // Si llegamos aquí, es un tipo desconocido.
        break;
    }

    return hash;
}
