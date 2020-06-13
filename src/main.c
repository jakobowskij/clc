/*
Created by Juliusz Jakobowski

A command line program that recieves a mathematical expression as input, evaluates the expression, and returns the result.
Implemented using a shunting yard algorithm modified to accept functions, implicit multiplication, and ability to distinguish
unary negation and binary subtraction.

Released under the MIT License.
*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include "constants.h"
#include "tokenize.h"
#include "auxiliary.h"
#include "variables.h"
#include "rpn.h"
#include "global.h"

typedef struct {
	char size;
	char dim[7];
} array_header;

// *(int*)(A+1);   To access multiple positions of char array as a single int
// variableMap[position] = malloc(sizeof(double));
// *((double*)variableMap[position]) = value;

char error;
char terminalInput[INPUT_SIZE];   // Entered by user
char unrecognizedToken[INPUT_HOLDER_SIZE]; // Printed to alert user of invalid input
unsigned int expressionRPN[RPN_SIZE];  // Printed to the terminal
double variableMap[VAR_MAP_SIZE];  // Memory for all variables, regardless of type or size
char variableNames[VAR_NAME_SIZE][10];
char variableTypes[VAR_MAP_SIZE];  // Stores type of each variable, or if space is currently unallocated

// User enters an expression as input.  It is evaluated and the result is returned, barring any errors
int main() {

	double printVal;  // Value resulting from computation
	bool scientificNotation = false;

	variableNames[0][0] = 'a'; variableNames[0][1] = 'n'; variableNames[0][2] = 's';
	loadVariables(CONST_START, USER_VAR_START, "consts.txt"); // Load constants

	printf("> ");
	while (fgets(terminalInput, INPUT_SIZE, stdin)) {

		if (terminalInput[INPUT_SIZE - 2] != '\0') {
			error = ERR_OVERFLOW;
		}
		
		// Perform calculation
		if (error == NO_ERROR) {
			inputToRPN();   
		}
		if (error == NO_ERROR) {
			printVal = evaluateRPN(); ////////// IN PROGRESS
		}

		// Print output, depending on errors and other conditions
		switch (error) {
		case NO_ERROR:
			if (scientificNotation) {
				printf("  %.15E\n", printVal);
			}
			else {
				printf("  %.*lf\n", findNumDecimals(printVal), printVal);
			}

			// Set "ans" to the latest result
			variableMap[ANS_ADDR] = printVal;

			break;
		case ERR_SYNTAX:
			printf("  Syntax error\n");
			break;
		case ERR_UNKNOWN_TOKEN:
			printf("  Unrecognized token \"");
			for (int i = 0; i < INPUT_HOLDER_SIZE-1; i++) {
				if (unrecognizedToken[i] == '\0') break;
				if (unrecognizedToken[i] == '\t') {
					printf("[tab]");
				}
				else {
					printf("%c", unrecognizedToken[i]);
				}
			}
			if (unrecognizedToken[INPUT_HOLDER_SIZE - 1] != 0) printf("...");
			printf("\n");
			break;
		case ERR_OVERFLOW:
			printf("  Overflow error\n");
			break;
		case ERR_UNDEFINED:
			printf("  Undefined or out of bounds\n");
			break;
		}

		printf("\n> ");
		resetValues(&printVal);
	}

	return 0;
}