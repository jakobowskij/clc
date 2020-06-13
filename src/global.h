#ifndef GLOBAL_H
#define GLOBAL_H

#include "constants.h"

extern double variableMap[VAR_MAP_SIZE];  // Memory for all variables, regardless of type or size
extern char variableNames[VAR_NAME_SIZE][10];
extern char variableTypes[VAR_MAP_SIZE];  // Stores type of each variable, or if space is currently unallocated
extern char terminalInput[INPUT_SIZE];   // Raw user input from terminal, \n\0 terminated
extern unsigned int expressionRPN[RPN_SIZE]; // Stores operations and variables in RPN format
extern char unrecognizedToken[INPUT_HOLDER_SIZE]; // Printed to alert user of invalid input
extern char error;

#endif