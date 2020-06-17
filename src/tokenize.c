#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include "constants.h"
#include "auxiliary.h"
#include "tokenize.h"
#include "global.h"
#include "variables.h"

unsigned int tokenize(int* indexPtr, int* evalVarHead, bool unaryNegation, int* keywordState) {
	// Converts multi-character inputs (such as function names) into their representative tokens

	unsigned int currChar = terminalInput[*indexPtr];
	char inputHolder[INPUT_HOLDER_SIZE] = { 0 }; int inputHolderIndex = 0;
	int numDecimalPlaces = 0; // If this exeeds one for a number, throw error
	int numE = 0; // Amount of times 'E' is encountered in a number
	bool prevCharIsE = false;
	bool negativeAnswerIndex = false;
	unsigned int outputToken = 0;
	int varMapHead = 0;  // variableMap indices beyond this value are guaranteed to be unallocated

	if ((currChar >= '0' && currChar <= '9')) {
		// If token is a value, scan all characters until no longer part of a value
		while ((currChar >= '0' && currChar <= '9') || currChar == '.' || currChar == 'E' || currChar == '-') {
			
			// Catches notation errors related to negation, decimal points, and scientific notation
			if (currChar == '-') {
				if (!prevCharIsE) {
					break;
				}
			}
			prevCharIsE = false;
			if (currChar == '.') {
				if (numE > 0) {
					error = ERR_SYNTAX;
					break;
				}
				numDecimalPlaces++;
			}
			else if (currChar == 'E') {
				numE++;
				prevCharIsE = true;
			}
			if (numE > 1) break;

			if (inputHolderIndex >= INPUT_HOLDER_SIZE) {
				error = ERR_OVERFLOW;
				break;
			}

			inputHolder[inputHolderIndex] = currChar;
			inputHolderIndex++;
			(*indexPtr)++;
			currChar = terminalInput[*indexPtr];
		}

		// Validate number, convert to double and store
		if (numDecimalPlaces > 1) return OP_NULL;
		if (*evalVarHead >= VAR_MAP_SIZE) {
			error = ERR_OVERFLOW;
			return OP_NULL;
		}
		variableMap[*evalVarHead] = atof(inputHolder);
		// TODO: Add support for different data types

		outputToken = *evalVarHead;
		(*evalVarHead)++;
	}
	else if ((currChar >= 'a' && currChar <= 'z') || (currChar >= 'A' && currChar <= 'Z')) {
		// If token is a function or variable
		while ((currChar >= 'a' && currChar <= 'z') || (currChar >= 'A' && currChar <= 'Z') || (currChar >= '0' && currChar <= '9')
			|| currChar == '_') {
			if (inputHolderIndex >= INPUT_HOLDER_SIZE) {
				break;
			}

			// Scan characters until no longer can be part of a function or variable
			inputHolder[inputHolderIndex] = currChar;
			(*indexPtr)++;
			inputHolderIndex++;
			currChar = terminalInput[*indexPtr];
		}

		outputToken = findFunction(inputHolder);

		// If token wasn't a function, test for variables
		if (outputToken == OP_NULL) {
			variableMap[*evalVarHead] = findVariable(inputHolder);
			outputToken = *evalVarHead;
			(*evalVarHead)++;
		}
	}
	else if (currChar == '<') {
		(*indexPtr)++;
		currChar = terminalInput[*indexPtr];

		if (currChar == '=') {
			outputToken = OP_LESS_THAN_EQUAL_TO;
			(*indexPtr)++;
		}
		else if (currChar == '-') {
			(*indexPtr)++;
			currChar = terminalInput[*indexPtr];

			if (currChar == '>') {
				outputToken = OP_IFF;
				(*indexPtr)++;
			}
			else {
				outputToken = OP_IMPLIED_BY;
			}
		}
		else if (currChar == '<') {
			outputToken = OP_LEFT_SHIFT;
			(*indexPtr)++;
		}
		else {
			outputToken = OP_LESS_THAN;
		}
	}
	else if (currChar == '>') {
		(*indexPtr)++;
		currChar = terminalInput[*indexPtr];

		if (currChar == '=') {
			outputToken = OP_GREATER_THAN_EQUAL_TO;
			(*indexPtr)++;
		}
		else if (currChar == '>') {
			outputToken = OP_RIGHT_SHIFT;
			(*indexPtr)++;
		}
		else {
			outputToken = OP_GREATER_THAN;
		}
	}
	else if (currChar == '-') {
		(*indexPtr)++;
		currChar = terminalInput[*indexPtr];

		if (currChar == '>') {
			outputToken = OP_IMPLIES;
			(*indexPtr)++;
		}
		else {
			outputToken = (unaryNegation) ? OP_NEG : OP_SUB;
		}
	}
	else {
		// If none of the above applied, single character tokens are tested for
		switch (currChar) {
		case '\n':
			outputToken = OP_NULL;
			break;
		case ' ':
			outputToken = OP_NULL;
			break;
		case ',':
			outputToken = ARG_SEPARATOR;
			break;
		case '(':
			outputToken = LEFT_PARENTH;
			break;
		case ')':
			outputToken = RIGHT_PARENTH;
			break;
		case '+':
			outputToken = OP_ADD;
			break;
		case '*':
			outputToken = OP_MUL;
			break;
		case '/':
			outputToken = OP_DIV;
			break;
		case '^':
			outputToken = OP_EXP;
			break;
		case '=':
			if (*keywordState == KWS_ASSIGN) {
				outputToken = INST_ASSIGN_VAL;
				*keywordState = KWS_NULL;
			}
			else {
				error = ERR_SYNTAX;
				return 0;
			}
			break;
		default:
			outputToken = OP_NULL;
			error = ERR_UNKNOWN_TOKEN;
			unrecognizedToken[0] = currChar;
			break;
		}
		(*indexPtr)++;
	}

	return outputToken;
}