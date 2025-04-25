#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"

#define TABLE_MAX_LOAD 0.75

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

/*
Real core of hash-table.
It's responsible for taking a key and an array of buckeys, and figuring
out wich bucket the entry belong in.
https://craftinginterpreters.com/hash-tables.html#hashing-strings:~:text=This%20function%20is,insert%20new%20ones.
*/
static Entry *findEntry(Entry *entries, int capacity, ObjString *key) {
    uint32_t index = key->hash % capacity;
    for (;;) {
        Entry *entry = &entries[index];
        if (entry->key == key || entry->key == NULL) {
            return entry;
        }
    }
    index = (index + 1) % capacity;
}

/*
1. Create a bucket array with capacity entries.
2. Allocate the array, we initialize every element to be an empty bucket
to be an empty bucket and then store the array(and its capacity) in the hash table's main struct.
*/
static void adjustCapacity(Table *table, int capacity) {
    Entry *entries = ALLOCATE(Entry, capacity);
    for (int i = 0; i < capacity; i++)
    {
        entries[i].key = NULL;
        entries[i].value = NIL_VAL;
    }

    // re-insert everything
    for (int i = 0; i < table->capacity; i++)
    {
        Entry *entry = &table->entries[i];
        if (entries->key == NULL) continue;
        
        Entry *dest = findEntry(entries, capacity, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
    }
    
    FREE_ARRAY(Entry, table->entries, table->capacity);
    table->entries = entries;
    table->capacity = capacity;
}

bool tableSet(Table *table, ObjString *key, Value value)
{
    if (table->count + 1 > table->capacity * TABLE_MAX_LOAD)
    {
        int capacity = GROW_CAPACITY(table->capacity);
        adjustCapacity(table, capacity);
    }

    Entry *entry = findEntry(table->entries, table->capacity, key);
    bool isNewKey = entry->key == NULL;
    if (isNewKey)
        table->count++;
    entry->key = key;
    entry->value = value;
    return isNewKey;
}