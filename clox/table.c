/**
 * @file table.c
 */

#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"

/** The maximum load factor for the table */
#define TABLE_MAX_LOAD 0.75

/**
 * Find the entry in the internal table representation
 * for the specified key.
 * @param entries The entries array
 * @param capacity The table capacity
 * @param key The key for which to search
 * @return The entry at which `key` appears
 */
static Entry* findEntry(Entry* entries, int capacity, StringObject* key) {
  // Compute the index at which to begin the search
  uint32_t index = key->hash % capacity;
  // Search linearly from the start index
  Entry* tombstone = NULL;
  for (;;) {
    Entry* entry = &entries[index];
    if (entry->key == NULL) {
      if (IS_NIL(entry->value)) {
        // Empty entry
        return (tombstone != NULL) ? tombstone : entry;
      } else {
        // Found a tombstone
        if (tombstone == NULL) tombstone = entry;
      }
      return entry;
    } else if (entry->key == key) {
      // Found the key of interest
      return entry;
    }
    index = (index + 1) % capacity;
  }
}

/**
 * Adjust the capacity of the table by allocating
 * a new dynamic array of entries.
 * @param table The table to adjust
 * @param capacity The desired table capacity
 */
static void adjustCapacity(Table* table, int capacity) {
  // Allocate and initialize a new table
  Entry* entries = ALLOCATE(Entry, capacity);
  for (int i = 0; i < capacity; ++i) {
    entries[i].key = NULL;
    entries[i].value = NIL_VAL;
  }

  // Copy existing entries to the new table
  table->count = 0;
  for (int i = 0; i < table->capacity; ++i) {
    Entry* entry = &table->entries[i];
    if (entry->key == NULL) continue;

    Entry* dst = findEntry(entries, capacity, entry->key);
    dst->key = entry->key;
    dst->value = entry->value;
    table->count++;
  }

  FREE_ARRAY(Entry, table->entries, table->capacity);
  table->entries = entries;
  table->capacity = capacity;
}

void initTable(Table* table) {
  table->count = 0;
  table->capacity = 0;
  table->entries = NULL;
}

void freeTable(Table* table) {
  FREE_ARRAY(Entry, table->entries, table->capacity);
  initTable(table);
}

bool putTable(Table* table, StringObject* key, Value value) {
  if (table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
    int capacity = GROW_CAPACITY(table->capacity);
    adjustCapacity(table, capacity);
  }

  Entry* entry = findEntry(table->entries, table->capacity, key);
  bool isNewKey = (entry->key == NULL);

  // Increment the count for the table ONLY
  // when the <key, value> pair is added at
  // a previously empty entry, not tombstone
  if (isNewKey && IS_NIL(entry->value)) table->count++;

  entry->key = key;
  entry->value = value;
  return isNewKey;
}

bool getTable(Table* table, StringObject* key, Value* value) {
  if (table->count == 0) return false;

  Entry* entry = findEntry(table->entries, table->capacity, key);
  if (entry->key == NULL) return false;

  *value = entry->value;
  return true;
}

bool delTable(Table* table, StringObject* key) {
  if (table->count == 0) return false;

  // Find the entry
  Entry* entry = findEntry(table->entries, table->capacity, key);
  if (entry->key == NULL) return false;

  // Place a tombstone in the entry
  entry->key = NULL;
  entry->value = BOOL_VAL(true);
  return true;
}

void copyTable(Table* src, Table* dst) {
  for (int i = 0; i < src->capacity; ++i) {
    Entry* entry = &src->entries[i];
    if (entry->key != NULL) {
      putTable(dst, entry->key, entry->value);
    }
  }
}

StringObject* findStringTable(Table* table, const char* data, int length, uint32_t hash) {
  if (table->count == 0) return NULL;

  uint32_t index = hash % table->capacity;

  // NOTE: This is the only place in the VM where we
  // actually compare strings for textual equality!

  for (;;) {
    Entry* entry = &table->entries[index];
    if (entry->key == NULL) {
      // Stop if we find an empty, non-tombstone entry
      if (IS_NIL(entry->value)) return NULL;
    } else if (entry->key->length == length 
            && entry->key->hash == hash
            && memcmp(entry->key->data, data, length) == 0) {
      // Found it
      return entry->key;
    }
    index = (index + 1) % table->capacity;
  }
}

void markForGCTable(Table* table) {
  for (int i = 0; i < table->capacity; ++i) {
    Entry* entry = &table->entries[i];
    markObject((Object*)entry->key);
    markValue(entry->value);
  }
}

void removeWeakRefsTable(Table* table) {
  for (int i = 0; i < table->capacity; ++i) {
    Entry* entry = &table->entries[i];
    if (entry->key != NULL && !entry->key->object.isMarked) {
      delTable(table, entry->key);
    }
  }
}