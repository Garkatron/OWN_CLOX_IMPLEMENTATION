#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"

#define TABLE_MAX_LOAD 0.75
#define VALUE_HASH(value) (getHash(value))

void initTable(Table *table)
{
    table->count = 0;
    table->capacity = 0;
    table->entries = NULL;
}

void freeTable(Table *table)
{
    FREE_ARRAY(Entry, table->entries, table->capacity);
    initTable(table);
}

static uint32_t hashNumber(const void *data, size_t length)
{
    uint32_t hash = 216613621u;
    const uint8_t *bytes = (const uint8_t *)data;
    for (int i = 0; i < length; i++)
    {
        hash ^= bytes[i];
        hash *= 16777619;
    }
    return hash;
}

static uint32_t getHash(Value value)
{
    switch (value.type)
    {
    case VAL_OBJ:
        switch (OBJ_TYPE(value))
        {
        case OBJ_STRING:
            return AS_STRING(value)->hash;

        default:
            return 0;
        }

    case VAL_NUMBER:
        return hashNumber(&value.as.number, sizeof(double));

    case VAL_BOOL:
        return AS_BOOL(value) ? 1231 : 1237;

    case VAL_NIL:
        return 2166136261u; 

    default:
        return 0;
    }
}


/*
Real core of hash-table.
It's responsible for taking a key and an array of buckeys, and figuring
out wich bucket the entry belong in.
https://craftinginterpreters.com/hash-tables.html#hashing-strings:~:text=This%20function%20is,insert%20new%20ones.
*/
static Entry *findEntry(Entry *entries, int capacity, Value key)
{

    // Gets the hash of the value or creates it
    uint32_t index = VALUE_HASH(key) % capacity;
    Entry *tombstone = NULL;

    for (;;)
    {
        Entry *entry = &entries[index];
        printValue(entry->key);

        if (IS_NIL(entry->key))
        {
            if (IS_NIL(entry->value))
            {
                return tombstone != NULL ? tombstone : entry;
            }
            else
            {
                if (tombstone == NULL)
                    tombstone = entry;
            }
        }
        else if (valuesEqual(entry->key, key))
        {
            return entry;
        }

        index = (index + 1) % capacity;
    }
}

bool tableGet(Table *table, Value key, Value *value)
{   
    if (table->count == 0)
        return false;

    Entry *entry = findEntry(table->entries, table->capacity, key);
    if (IS_NIL(entry->key))
        return false;


    *value = entry->value;
    return true;
}

/*
1. Create a bucket array with capacity entries.
2. Allocate the array, we initialize every element to be an empty bucket
to be an empty bucket and then store the array(and its capacity) in the hash table's main struct.
*/
static void adjustCapacity(Table *table, int capacity)
{
    Entry *entries = ALLOCATE(Entry, capacity);
    for (int i = 0; i < capacity; i++)
    {
        entries[i].key = NIL_VAL;
        entries[i].value = NIL_VAL;
    }

    // Reinsert existing entries into the new array
    table->count = 0;
    for (int i = 0; i < table->capacity; i++)
    {
        Entry *entry = &table->entries[i];
        if (IS_NIL(entry->key))
            continue;

        Entry *dest = findEntry(entries, capacity, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
        table->count++;
    }

    FREE_ARRAY(Entry, table->entries, table->capacity);
    table->entries = entries;
    table->capacity = capacity;
}

bool tableSet(Table *table, Value key, Value value)
{
    if (table->count + 1 > table->capacity * TABLE_MAX_LOAD)
    {
        int capacity = GROW_CAPACITY(table->capacity);
        adjustCapacity(table, capacity);
    }

    Entry *entry = findEntry(table->entries, table->capacity, key);
    bool isNewKey = IS_NIL(entry->key);
    if (isNewKey && IS_NIL(entry->value))
    {
        table->count++;
    }

    entry->key = key;
    entry->value = value;
    return isNewKey;
}

bool tableDelete(Table *table, Value *key)
{
    if (table->count == 0)
        return false;

    // Find the entry
    Entry *entry = findEntry(table->entries, table->capacity, *key);
    if (IS_NIL(entry->key))
        return false;

    // Place a tombstone in the entry.
    entry->key = NIL_VAL;
    entry->value = BOOL_VAL(true);
    return true;
}

void tableAddAll(Table *from, Table *to)
{
    for (int i = 0; i < from->capacity; i++)
    {
        Entry *entry = &from->entries[i];
        if (!IS_NIL(entry->key))
        {
            tableSet(to, entry->key, entry->value);
        }
    }
}
/*
It appears we have copy-pasted findEntry().
There is a lot of redundancy, but also a couple of key differences.
First, we pass in the raw character array of the key we’re looking for instead of an ObjString.
At the point that we call this, we haven’t created an ObjString yet.

Second, when checking to see if we found the key, we look at the actual strings.
We first see if they have matching lengths and hashes.
Those are quick to check and if they aren’t equal, the strings definitely aren’t the same.

If there is a hash collision, we do an actual character-by-character string comparison.
This is the one place in the VM where we actually test strings for textual equality.
We do it here to deduplicate strings and then the rest of the VM can take for granted that any two strings at different addresses in memory must have different contents.

*/
// Busca cualquier valor en la tabla.
// Retorna un puntero al valor internado, o NULL si no existe.
Value *tableFindValue(Table *table, Value *key)
{
    if (table->count == 0)
        return NULL;

    uint32_t hash = VALUE_HASH(*key);
    uint32_t index = hash % table->capacity;

    for (;;)
    {
        Entry *entry = &table->entries[index];

        if (IS_NIL(entry->key))
        {
            if (IS_NIL(entry->value))
            {
                return NULL; // Slot vacío, sin tombstone: no encontrado.
            }
        }
        else
        {
            Value candidate = entry->key;
            if (candidate.type == key->type)
            {
                switch (key->type)
                {
                case VAL_BOOL:
                    if (AS_BOOL(candidate) == AS_BOOL(*key))
                        return &entry->key;
                    break;

                case VAL_NIL:
                    return &entry->key; // Solo hay un valor nil.

                case VAL_NUMBER:
                    if (AS_NUMBER(candidate) == AS_NUMBER(*key))
                        return &entry->key;
                    break;

                case VAL_OBJ:
                    if (OBJ_TYPE(candidate) == OBJ_STRING &&
                        OBJ_TYPE(*key) == OBJ_STRING)
                    {
                        ObjString *candStr = AS_STRING(candidate);
                        ObjString *keyStr = AS_STRING(*key);
                        if (candStr->length == keyStr->length &&
                            candStr->hash == keyStr->hash)
                        {
                            if (memcmp(candStr->chars, keyStr->chars, candStr->length) == 0)
                                return &entry->key;
                        }
                    }
                    break;

                default:
                    break;
                }
            }
        }

        index = (index + 1) % table->capacity;
    }
}

ObjString *tableFindString(Table *table, const char *chars, int length, uint32_t hash)
{
    if (table->count == 0)
        return NULL;

    uint32_t index = hash % table->capacity;
    for (;;)
    {
        Entry *entry = &table->entries[index];
        if (IS_NIL(entry->key))
        {
            // Stop if we find an empty non-tombstone entry.
            if (IS_NIL(entry->value))
                return NULL;
        }
        if (IS_OBJ(entry->key) && OBJ_TYPE(entry->key) == OBJ_STRING)
        {
            ObjString *candStr = AS_STRING(entry->key);
            if (candStr->length == length &&
                candStr->hash == hash)
            {
                const char *candChars = candStr->chars;

                if (memcmp(chars, candChars, candStr->length) == 0)
                    return candStr; // We found it.
            }
        }

        index = (index + 1) % table->capacity;
    }
}

void tablePrintContent(Table *table) {
    printf("\n");
    printf("<-----------{ %s }----------->\n", "Table");
    for (int i = 0; i < table->capacity; i++) {
        Entry entry = table->entries[i];
        if (IS_NIL(entry.key)) continue;
        printf("Key:");
        printValue(entry.key);
        printf("\n");
        printf("Value:");
        printValue(entry.value);
        printf("\n");

    }
}
