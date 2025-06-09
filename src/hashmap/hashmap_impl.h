#ifndef HASHMAP_IMPL_H
#define HASHMAP_IMPL_H

#ifndef HASHMAP_NAME
#error "HASHMAP_NAME must be defined before including hashmap_impl.h"
#endif

#ifndef KEY_TYPE
#error "KEY_TYPE must be defined"
#endif

#ifndef VALUE_TYPE
#error "VALUE_TYPE must be defined"
#endif

#ifndef HASH_FUNC
#error "HASH_FUNC must be defined"
#endif

#ifndef EQUALS_FUNC
#error "EQUALS_FUNC must be defined"
#endif

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define HASHMAP_MAX_LOAD 0.75

// Macros de concatenación
#define CONCAT(a, b) a##b
#define MAKE_ENTRY(name) CONCAT(name, _Entry)
#define FUNC(name, method) CONCAT(name, _##method)

// Definición del struct de entrada
typedef struct MAKE_ENTRY(HASHMAP_NAME)
{
    KEY_TYPE key;
    VALUE_TYPE value;
} MAKE_ENTRY(HASHMAP_NAME);

// Definición del struct del hashmap
typedef struct
{
    int count;
    int capacity;
    VALUE_TYPE *vLast;
    KEY_TYPE *kLast;
    uint32_t (*hashFunc)(const void *);
    bool (*equalsFunc)(const void *, const void *);
    void (*freeKey)(void *key);
    void (*freeValue)(void *value);
    MAKE_ENTRY(HASHMAP_NAME) * entries;
} HASHMAP_NAME;

// findEntry
static MAKE_ENTRY(HASHMAP_NAME) * FUNC(HASHMAP_NAME, findEntry)(
                               MAKE_ENTRY(HASHMAP_NAME) * entries,
                               int capacity,
                               const void *key,
                               uint32_t (*hashFunc)(const void *),
                               bool (*equalsFunc)(const void *, const void *))
{
    uint32_t index = hashFunc(key) % capacity;
    MAKE_ENTRY(HASHMAP_NAME) *tombstone = NULL;

    for (;;)
    {
        MAKE_ENTRY(HASHMAP_NAME) *entry = &entries[index];
        if (entry->key == NULL)
        {
            if (entry->value == NULL)
            {
                return tombstone != NULL ? tombstone : entry;
            }
            else if (tombstone == NULL)
            {
                tombstone = entry;
            }
        }
        else if (equalsFunc(entry->key, key))
        {
            return entry;
        }
        index = (index + 1) % capacity;
    }
}

// adjustCapacity
void FUNC(HASHMAP_NAME, adjustCapacity)(HASHMAP_NAME *map, int capacity)
{
    map->kLast = NULL;
    map->vLast = NULL;

    MAKE_ENTRY(HASHMAP_NAME) *entries = malloc(sizeof(MAKE_ENTRY(HASHMAP_NAME)) * capacity);
    for (int i = 0; i < capacity; i++)
    {
        entries[i].key = NULL;
        entries[i].value = NULL;
    }

    map->count = 0;
    for (int i = 0; i < map->capacity; i++)
    {
        MAKE_ENTRY(HASHMAP_NAME) *entry = &map->entries[i];
        if (entry->key == NULL)
            continue;

        MAKE_ENTRY(HASHMAP_NAME) *dest = FUNC(HASHMAP_NAME, findEntry)(
            entries, capacity, entry->key, map->hashFunc, map->equalsFunc);
        dest->key = entry->key;
        dest->value = entry->value;
        map->count++;
    }

    free(map->entries);
    map->entries = entries;
    map->capacity = capacity;
}

// init
void FUNC(HASHMAP_NAME, init)(
    HASHMAP_NAME *map,
    uint32_t (*hashFunc)(const void *),
    bool (*equalsFunc)(const void *, const void *),
    void (*freeKey)(void *),
    void (*freeValue)(void *))
{
    map->count = 0;
    map->capacity = 0;
    map->entries = NULL;
    map->kLast = NULL;
    map->vLast = NULL;
    map->hashFunc = hashFunc;
    map->equalsFunc = equalsFunc;
    map->freeKey = freeKey;
    map->freeValue = freeValue;
}

// free
void FUNC(HASHMAP_NAME, free)(HASHMAP_NAME *map)
{
    if (map->freeKey || map->freeValue)
    {
        for (int i = 0; i < map->capacity; i++)
        {
            MAKE_ENTRY(HASHMAP_NAME) *entry = &map->entries[i];
            if (entry->key && map->freeKey)
                map->freeKey(entry->key);
            if (entry->value && map->freeValue)
                map->freeValue(entry->value);
        }
    }
    free(map->entries);
    FUNC(HASHMAP_NAME, init)(map, map->hashFunc, map->equalsFunc, map->freeKey, map->freeValue);
}

// set
bool FUNC(HASHMAP_NAME, set)(HASHMAP_NAME *map, KEY_TYPE key, VALUE_TYPE value)
{
    if (map->count + 1 > map->capacity * HASHMAP_MAX_LOAD)
    {
        int newCap = map->capacity < 8 ? 8 : map->capacity * 2;
        FUNC(HASHMAP_NAME, adjustCapacity)(map, newCap);
    }

    MAKE_ENTRY(HASHMAP_NAME) *entry = FUNC(HASHMAP_NAME, findEntry)(
        map->entries, map->capacity, key, map->hashFunc, map->equalsFunc);
    bool isNew = (entry->key == NULL && entry->value == NULL);
    if (isNew)
        map->count++;

    entry->key = key;
    entry->value = value;

    map->kLast = &entry->key;
    map->vLast = &entry->value;

    return isNew;
}

// get
bool FUNC(HASHMAP_NAME, get)(HASHMAP_NAME *map, const void *key, VALUE_TYPE *valueOut)
{
    if (map->count == 0)
        return false;

    if (map->kLast != NULL && map->equalsFunc(map->kLast, key))
    {
        *valueOut = *map->vLast;
        return true;
    }

    MAKE_ENTRY(HASHMAP_NAME) *entry = FUNC(HASHMAP_NAME, findEntry)(
        map->entries, map->capacity, key, map->hashFunc, map->equalsFunc);
    if (entry->key == NULL)
        return false;

    *valueOut = entry->value;
    return true;
}

// delete
bool FUNC(HASHMAP_NAME, delete)(HASHMAP_NAME *map, const void *key)
{
    if (map->count == 0)
        return false;

    MAKE_ENTRY(HASHMAP_NAME) *entry = FUNC(HASHMAP_NAME, findEntry)(
        map->entries, map->capacity, key, map->hashFunc, map->equalsFunc);
    if (entry->key == NULL)
        return false;

    if (map->kLast != NULL && map->equalsFunc(map->kLast, key))
    {
        map->vLast = NULL;
        map->kLast = NULL;
    }

    if (map->freeKey)
        map->freeKey(entry->key);
    if (map->freeValue)
        map->freeValue(entry->value);

    entry->key = NULL;
    entry->value = (void *)1; // Tombstone
    map->count--;

    return true;
}

// debugPrint
void FUNC(HASHMAP_NAME, debugPrint)(HASHMAP_NAME *map)
{
    printf("HashMap content:\n");
    for (int i = 0; i < map->capacity; i++)
    {
        MAKE_ENTRY(HASHMAP_NAME)
        entry = map->entries[i];
        if (entry.key && entry.value)
            printf("  [%d] key=%p, value=%p\n", i, entry.key, entry.value);
    }
}

int FUNC(HASHMAP_NAME, count)(HASHMAP_NAME *map)
{
    int count = 0;
    for (int i = 0; i < map->capacity; i++)
    {
        MAKE_ENTRY(HASHMAP_NAME) entry = map->entries[i];
        if (entry.key && entry.value)
            count++;
    }
    return count;
}
HASHMAP_NAME* FUNC(HASHMAP_NAME, new)(
    uint32_t (*hashFunc)(const void *),
    bool (*equalsFunc)(const void *, const void *),
    void (*freeKey)(void *),
    void (*freeValue)(void *)
) {
    HASHMAP_NAME* map = malloc(sizeof(HASHMAP_NAME));
    FUNC(HASHMAP_NAME, init)(map, hashFunc, equalsFunc, freeKey, freeValue);
    return map;
}


#endif // HASHMAP_IMPL_H
