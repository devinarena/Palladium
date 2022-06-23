
/**
 * @file constant.h
 * @author Devin Arena
 * @brief Logic for constants in the program. A constant can be an integer,
 * boolean, double, or string.
 * @since 6/22/2022
 **/

#ifndef PALLADIUM_CONSTANT_H
#define PALLADIUM_CONSTANT_H

#define IS_BOOLEAN(constant) (constant->type == CONSTANT_BOOLEAN)
#define IS_INTEGER(constant) (constant->type == CONSTANT_INTEGER)
#define IS_DOUBLE(constant) (constant->type == CONSTANT_DOUBLE)

#define TO_BOOLEAN(constant) (constant->value.boolean)
#define TO_INTEGER(constant) (constant->value.integer)
#define TO_DOUBLE(constant) (constant->value.double_)

typedef enum {
  CONSTANT_INTEGER,
  CONSTANT_DOUBLE,
  CONSTANT_BOOLEAN
} ConstantType;

typedef struct {
  union {
    bool boolean;
    int integer;
    double double_;
  } value;
  ConstantType type;
} Constant;

#endif