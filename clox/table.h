/**
 * @file table.h
 */

#ifndef CLOX_TABLE_H
#define CLOX_TABLE_H

#include "common.h"
#include "value.h"

/**
 * The Entry type represents a single entry in the table.
 */
typedef struct {
  /** The entry key */
  StringObject* key;
  /** The entry value */
  Value value;
} Entry;

/**
 * The Table type implements an open-addressing hashmap.
 */
typedef struct {
  /** The total number of entries in the table */
  int count;
  /** The total capacity of the table */
  int capacity;
  /** The bucket array */
  Entry* entries;
} Table;

/**
 * Initialize the table data structure.
 * @param table The table to initialize
 */
void initTable(Table* table);

/**
 * Release all of the resources owned by `table`.
 * @param table The table to release
 */
void freeTable(Table* table);

/**
 * Put the specified <key, value> pair into the table,
 * overwriting an existing <key, value> pair if present.
 * @param table The table on which the put is performed
 * @param key The key
 * @param value The value
 * @return `true` if the new entry is added, `false` otherwise
 */
bool putTable(Table* table, StringObject* key, Value value);

/**
 * Query the table for the entry identified by `key`.
 * @param table The table on which the get is performed
 * @param key The key
 * @param[out] value The retrieved value
 * @param `true` if the key is present in the table, `false` otherwise
 */
bool getTable(Table* table, StringObject* key, Value* value);

/**
 * Remove the entry identified by `key` from the table, if present.
 * @param table The table on which the delete is performed
 * @param key The key
 * @return `true` if the entry is removed, `false` otherwise
 */
bool delTable(Table* table, StringObject* key);

/**
 * Copy all of the entries from `src` into `dst`.
 * @param src The source table
 * @param dst The destination table
 */
void copyTable(Table* src, Table* dst);

/**
 * Search for the specified string in the table.
 * @param table The table in which the search is performed
 * @param data The string data
 * @param length The string length
 * @param hash The string hash value
 * @return The string object if present, NULL otherwise
 */
StringObject* findStringTable(Table* table, const char* data, int length, uint32_t hash);

#endif // CLOX_TABLE_H