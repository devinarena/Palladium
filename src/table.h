
/**
 * @file table.h
 * @author Devin Arena
 * @brief header for hash table implementation.
 * @since 6/24/2022
 **/

#ifndef PALLADIUM_TABLE_H
#define PALLADIUM_TABLE_H

#include "commons.h"
#include "object.h"
#include "value.h"

typedef struct {
  PdString* key;
  Value value;
} Entry;

typedef struct {
  int count;
  int capacity;
  Entry* entries;
} Table;

void initTable(Table* table);
void freeTable(Table* table);
bool tableGet(Table* table, PdString* key, Value* value);
bool tableSet(Table* table, PdString* key, Value value);
bool tableDelete(Table* table, PdString* key);
void tableAddAll(Table* from, Table* to);
PdString* tableFindString(Table* table,
                           const char* chars,
                           int length,
                           uint32_t hash);
// void tableRemoveWhite(Table* table);
// void markTable(Table* table);

#endif