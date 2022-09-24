
/**
 * @file value.c
 * @author Devin Arena
 * @brief Handles logic for values.
 * @since 6/24/2022
 **/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "object.h"
#include "value.h"

/**
 * @brief Helper method for printing the specified value to standard output.
 *
 * @param constant Value* a pointer to the constant to print
 * @param type PrimitiveType the type of the constant
 */
void printValue(Value value) {
  switch (value.type) {
    case VALUE_NULL:
      printf("null");
      break;
    case VALUE_INTEGER:
      printf("%d", value.data.integer);
      break;
    case VALUE_DOUBLE:
      printf("%f", value.data.double_);
      break;
    case VALUE_BOOL:
      printf("%s", value.data.boolean ? "true" : "false");
      break;
    case VALUE_CHARACTER:
      printf("%c", value.data.character);
      break;
    case VALUE_POINTER:
      printf("%p", value.data.pointer);
      break;
    case VALUE_OBJECT:
      printObject(value);
      break;
    default:
      printf("Unknown value type\n");
      exit(1);
      break;
  }
}

/**
 * @brief toString method for values (builtin function asstr)
 *
 * @param value Value* the value to convert to a string
 * @return PdString* the string representation of the value
 */
PdString* toString(Value* value) {
  switch (value->type) {
    case VALUE_NULL:
      return copyString("null", 4);
    case VALUE_INTEGER: {
      int enough = ceil(log10(1 + abs(value->data.integer))) + 1;
      char* str = malloc(sizeof(char) * enough);
      sprintf(str, "%d", TO_INTEGER((*value)));
      return copyString(str, enough);
    }
    case VALUE_DOUBLE: {
      int enough = ceil(log10(1 + abs(value->data.double_))) + 1;
      char* str = malloc(sizeof(char) * enough);
      sprintf(str, "%f", value->data.double_);
      return copyString(str, enough);
    }
    case VALUE_BOOL:
      return copyString(value->data.boolean ? "true" : "false", 4);
    case VALUE_CHARACTER: {
      char* str = malloc(sizeof(char) * 2);
      str[0] = value->data.character;
      str[1] = '\0';
      return copyString(str, 2);
    }
    case VALUE_POINTER:
      return copyString("pointer", 7);
    case VALUE_OBJECT:
      return copyString("object", 6);
    default:
      return copyString("Unknown value type", 18);
  }
}

/**
 * @brief Get the the name of a value type.
 *
 * @param type the type of the value
 * @return const char* the name of the value type
 */
const char* getValueTypeName(ValueType type) {
  switch (type) {
    case VALUE_NULL:
      return "null";
    case VALUE_INTEGER:
      return "integer";
    case VALUE_DOUBLE:
      return "double";
    case VALUE_BOOL:
      return "boolean";
    case VALUE_CHARACTER:
      return "character";
    case VALUE_POINTER:
      return "pointer";
    case VALUE_OBJECT:
      return "object";
    default:
      return "unknown";
  }
}

/**
 * @brief Helper for comparing two values.
 *
 * @param a Value the first value to compare
 * @param b Value the second value to compare
 * @return bool true if the values are equal, false otherwise
 */
bool valuesEqual(Value a, Value b) {
  if ((a.type == VALUE_INTEGER || a.type == VALUE_DOUBLE) &&
      (b.type == VALUE_INTEGER || b.type == VALUE_DOUBLE)) {
    double av = a.type == VALUE_DOUBLE ? a.data.double_ : a.data.integer;
    double bv = b.type == VALUE_DOUBLE ? b.data.double_ : b.data.integer;
    return av == bv;
  }

  if (a.type != b.type) {
    return false;
  }

  switch (a.type) {
    case VALUE_BOOL:
      return a.data.boolean == b.data.boolean;
    case VALUE_CHARACTER:
      return a.data.character == b.data.character;
    default:
      return false;
  }
}