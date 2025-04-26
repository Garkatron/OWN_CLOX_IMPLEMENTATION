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
static Entry *findEntry(Entry *entries, int capacity, ObjString *key)
{
    uint32_t index = key->hash % capacity;
    Entry *tombstone = NULL;
    for (;;)
    {
        Entry *entry = &entries[index];
        if (entry->key == NULL)
        {
            if (IS_NIL(entry->value))
            {
                // Empty entry.
                return tombstone != NULL ? tombstone : entry;
            }
            else
            {
                // We found a tombstone.
                if (tombstone == NULL)
                    tombstone = entry;
            }
        }
        else if (entry->key == key)
        {
            return entry;
        }
    }
    index = (index + 1) % capacity;
}

bool tableGet(Table *table, ObjString *key, Value *value)
{
    if (table->count == 0)
        return false;
    Entry *entry = findEntry(table->entries, table->capacity, key);
    if (entry->key == NULL)
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
        entries[i].key = NULL;
        entries[i].value = NIL_VAL;
    }

    // re-insert everything
    table->count = 0;
    for (int i = 0; i < table->capacity; i++)
    {
        Entry *entry = &table->entries[i];
        if (entries->key == NULL)
            continue;

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
    if (isNewKey && IS_NIL(entry->value))
        table->count++;
    entry->key = key;
    entry->value = value;
    return isNewKey;
}

bool tableDelete(Table *table, ObjString *key)
{
    if (table->count == 0)
        return false;

    // Find the entry
    Entry *entry = findEntry(table->entries, table->capacity, key);
    if (entry->key == NULL)
        return false;

    // Place a tombstone in the entry.
    entry->key = NULL;
    entry->value = BOOL_VAL(true);
    return true;
}
void tableAddAll(Table *from, Table *to)
{
    for (int i = 0; i < from->capacity; i++)
    {
        Entry *entry = &from->entries[i];
        if (entry->key != NULL)
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
ObjString *tableFindString(Table *table, const char *chars, int length, uint32_t hash)
{
    if (table->count == 0)
        return NULL;
    uint32_t index = hash % table->capacity;
    for (;;)
    {
        Entry *entry = &table->entries[index];
        if (entry->key == NULL)
        {
            // Stop if we find an empty no-tombstone entry.
            if (IS_NIL(entry->value))
                return;
        }
        else if (entry->key->length == length && entry->key->hash == hash)
        {
            
            const char* keyChars = entry->key->ownsChars ? entry->key->as.chars : entry->key->as.strPtr;
            if (memcmp(keyChars, chars, length) == 0) return entry->key; // We found it.
            
        }
    }
}