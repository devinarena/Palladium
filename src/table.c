
/**
 * @file table.c
 * @author Devin Arena
 * @brief Implementation file for hash table.
 * @since 5/26/2022
 **/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"

// max load before table resizes
#define TABLE_MAX_LOAD 0.75

/**
 * @brief Initializes a table by zeroing out all of its memory.
 *
 * @param table Table* the table to initialize.
 */
void initTable(Table* table) {
  table->count = 0;
  table->capacity = 0;
  table->entries = NULL;
}

/**
 * @brief Frees a table by freeing its entries and zeroing its memory.
 *
 * @param table Table* the table to free.
 */
void freeTable(Table* table) {
  FREE_ARRAY(Entry, table->entries, table->capacity);
  initTable(table);
}

/**
 * @brief Takes the hash of the key and searches the array for it, if it doesnt
 * find it, it linear searches from that position.
 *
 * @param entries Entry* the array of entries to search.
 * @param capacity int the capacity of the array.
 * @param key ObjString* the key to search for.
 * @return Entry* the entry that was found.
 */
static Entry* findEntry(Entry* entries, int capacity, ObjString* key) {
  uint32_t index = key->hash & (capacity - 1);
  Entry* tombstone = NULL;

  while (true) {
    Entry* entry = &entries[index];

    if (entry->key == NULL) {
      if (IS_NULL(entry->value)) {
        return tombstone != NULL ? tombstone : entry;
      } else {
        if (tombstone == NULL)
          tombstone = entry;
      }
    } else if (entry->key == key) {
      return entry;
    }

    index = (index + 1) & (capacity - 1);
  }
}

/**
 * @brief Adjusts the capacity of the table by creating a new entries array and
 * copying the old entries into it.
 *
 * @param table Table* the table to adjust.
 * @param capacity int the new capacity of the table.
 */
static void adjustCapacity(Table* table, int capacity) {
  Entry* entries = ALLOCATE(Entry, capacity);
  for (int i = 0; i < capacity; i++) {
    entries[i].key = NULL;
    entries[i].value = NULL_VAL;
  }

  table->count = 0;
  for (int i = 0; i < table->capacity; i++) {
    Entry* entry = &table->entries[i];
    if (entry->key == NULL)
      continue;

    Entry* dest = findEntry(entries, capacity, entry->key);
    dest->key = entry->key;
    dest->value = entry->value;
    table->count++;
  }

  FREE_ARRAY(Entry, table->entries, table->capacity);

  table->entries = entries;
  table->capacity = capacity;
}

/**
 * @brief Gets a value from the table by searching for the key. Assigns the
 * value to the value pointer. If the key is not found, it returns false.
 *
 * @param table Table* the table to search.
 * @param key ObjString* the key to search for.
 * @param value Value* the value to assign to.
 * @return bool true if the key was found, false otherwise.
 */
bool tableGet(Table* table, ObjString* key, Value* value) {
  if (table->count == 0)
    return false;

  Entry* entry = findEntry(table->entries, table->capacity, key);
  if (entry->key == NULL)
    return false;

  *value = entry->value;
  return true;
}

/**
 * @brief Sets a value in the table, incrementing the count if the key is not in
 * the table. Adjusts the capacity if the load factor is greater than the max
 * load.
 *
 * @param table Table* the table to set the value in.
 * @param key ObjString* the key to set the value for.
 * @param value Value the value to set.
 * @return bool true if the key is not found, false otherwise.
 */
bool tableSet(Table* table, ObjString* key, Value value) {
  if (table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
    int capacity = GROW_CAPACITY(table->capacity);
    adjustCapacity(table, capacity);
  }

  Entry* entry = findEntry(table->entries, table->capacity, key);
  bool isNewKey = entry->key == NULL;
  if (isNewKey && IS_NULL(entry->value))
    table->count++;

  entry->key = key;
  entry->value = value;
  return isNewKey;
}

/**
 * @brief Deletes an entry from the table by setting it to be a tombstone.
 *
 * @param table Table* the table to delete the entry from.
 * @param key ObjString* the key to delete.
 * @return bool true if the key was found, false otherwise.
 */
bool tableDelete(Table* table, ObjString* key) {
  if (table->count == 0)
    return false;

  Entry* entry = findEntry(table->entries, table->capacity, key);
  if (entry->key == NULL)
    return false;

  entry->key = NULL;
  entry->value = FROM_BOOL(true);
  return true;
}

/**
 * @brief Copies all entires from one table to another.
 *
 * @param from Table* the table to copy from.
 * @param to Table* the table to copy to.
 */
void tableAddAll(Table* from, Table* to) {
  for (int i = 0; i < from->capacity; i++) {
    Entry* entry = &from->entries[i];
    if (entry->key == NULL)
      continue;

    tableSet(to, entry->key, entry->value);
  }
}

/**
 * @brief Searches the table for a string with the specified hash, characters,
 * and length.
 *
 * @param table Table* the table to search.
 * @param chars char* the characters to search for.
 * @param length int the length of the string.
 * @param hash uint32_t the hash of the string.
 * @return ObjString* the string that was found or NULL.
 */
ObjString* tableFindString(Table* table,
                           const char* chars,
                           int length,
                           uint32_t hash) {
  if (table->count == 0)
    return NULL;

  uint32_t index = hash & (table->capacity - 1);

  while (true) {
    Entry* entry = &table->entries[index];

    if (entry->key == NULL) {
      if (IS_NULL(entry->value))
        return NULL;
    } else if (entry->key->length == length && entry->key->hash == hash &&
               memcmp(entry->key->chars, chars, length) == 0) {
      return entry->key;
    }

    index = (index + 1) & (table->capacity - 1);
  }
}

// void tableRemoveWhite(Table* table) {
//   for (int i = 0; i < table->capacity; i++) {
//     Entry* entry = &table->entries[i];
//     if (entry->key != NULL && !entry->key->obj.isMarked) {
//       tableDelete(table, entry->key);
//     }
//   }
// }

// void markTable(Table* table) {
//   for (int i = 0; i < table->capacity; i++) {
//     Entry* entry = &table->entries[i];
//     markObject((Object*)entry->key);
//     markValue(entry->value);
//   }
// }