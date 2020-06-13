#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include<stdbool.h>
#include"constants.h"
#include "auxiliary.h"
#include "global.h"

int findNumDecimals(double input) {
	// Finds appropriate amount of decimals to display such that final output is accurate, and takes up similar amount of space each time
	int decimals = 15;
	while (input >= 10 && decimals > 0) {
		input /= 10.0;
		decimals--;
	}
	return decimals;
}

unsigned int pop(unsigned int arr[], int* length) {
	// Removes LAST element from array, returns that element, and decrements array length
	if (*length > 0) {
		unsigned int val = arr[*length - 1];
		arr[*length - 1] = 0;
		(*length)--;
		return val;
	}
	return OP_NULL;
}

void push(unsigned int arr[], unsigned int val, int* length, int maxLength) {
	// Appends input to the end of array, increments array length
	if (*length >= maxLength) {
		error = ERR_OVERFLOW;
		return;
	}
	arr[*length] = val;
	(*length)++;
}

bool stackIsEmpty(unsigned int stack[]) {
	// Returns true if given stack array is empty
	return (stack[0] == 0);
}

bool isFunction(unsigned int token) {
	// Returns true if given operator uses function notation, func(arg1, arg2)
	return ((token > END_OPS && token < END_FUNCS) || token > USER_FUNC_START);
}

bool isOperator(unsigned int token) {
	// Returns true if given token is an operator
	return (token >= OP_NULL && token < LEFT_PARENTH);
}

bool isBinaryOperator(unsigned int token) {
	// Returns true if function or operator has two inputs, false if one
	return (token < UNARY_OPERATORS && token != OP_NOT && token != OP_NEG && token > OP_NULL);
}

long long int doubleToInt(double input) {
	// Converts double to long long int
	return (long long int)((input >= 0) ? input + 0.5 : input - 0.5);
}

double gcd(double a, double b) {
	// Uses the Euclidean algorithm to find the greatest commoon denominator of two numbers
	long long int A = doubleToInt(a);
	long long int B = doubleToInt(b);
	long long int swap;

	while (B != 0) {
		swap = B;
		B = A % B;
		A = swap;
	}

	return (double)A;
}

unsigned int findFunction(char input[]) {
	// Returns the op-code for a given string representing a function. Unused (terminal) characters must be null

	// TODO: User defined functions

	char functions[NR_FUNCTIONS][10] = { "div", "mod", "log", "root",
		"sin", "cos", "tan", "sec", "csc", "cot",
		"asin", "acos", "atan", "asec", "acsc", "acot",
		"sinh", "cosh", "tanh", "sech", "csch", "coth",
		"asinh", "acosh", "atanh", "asech", "acsch", "acoth",
		"sqrt", "ln", "log10", "ceil", "floor", "round", "sgn", "gcd", "lcm", "atan2", "abs", "log2",
		"cbrt", "trunc", "erf", "erfc", "gamma", "hypot", "lgamma", "sinc", "nsinc", "reqll", "perr", "deg", "rad",
		"is", "and", "or", "not", "mod", "xor", "iff", "AND", "OR", "XOR", "NOT",
		"if", "elif", "else", "switch", "case", "while", "for", "goto", "break", "continue", "def", "class", "return", "del"
	};
	int functionOP[NR_FUNCTIONS] = { OP_DIV_INT, OP_MOD, OP_LOG, OP_ROOT,
		OP_SIN, OP_COS, OP_TAN, OP_SEC, OP_CSC, OP_COT,
		OP_ASIN, OP_ACOS, OP_ATAN, OP_ASEC, OP_ACSC, OP_ACOT,
		OP_SINH, OP_COSH, OP_TANH, OP_SECH, OP_CSCH, OP_COTH,
		OP_ASINH, OP_ACOSH, OP_ATANH, OP_ASECH, OP_ACSCH, OP_ACOTH,
		OP_SQRT, OP_LN, OP_LOG10, OP_CEIL, OP_FLOOR, OP_ROUND, OP_SIGN, OP_GCD, OP_LCM, OP_ATAN2, OP_ABS, OP_LOG2,
		OP_CBRT, OP_TRUNC, OP_ERF, OP_ERFC, OP_GAMMA, OP_HYPOT, OP_LGAMMA, OP_SINC, OP_NSINC, OP_REQLL, OP_PERR, OP_DEG, OP_RAD,
		OP_IS, OP_AND, OP_OR, OP_NOT, OP_MOD, OP_XOR, OP_IFF, OP_BITWISE_AND, OP_BITWISE_OR, OP_BITWISE_XOR, OP_BITWISE_NOT,
		KW_IF, KW_ELIF, KW_ELSE, KW_SWITCH, KW_CASE, KW_WHILE, KW_FOR, KW_GOTO, KW_BREAK, KW_CONTINUE, KW_DEF, KW_CLASS, KW_RETURN, KW_DEL
	};

	for (int i = 0; i < NR_FUNCTIONS; i++) { // function loop
		for (int j = 0; j < 10; j++) { // character loop
			if (input[j] != functions[i][j]) {
				// If at any point the input and current function don't match, try next function
				break;
			}
			else if (j == 9 && input[10] == '\0') {
				// Return if end of the input has been reached and all have matched
				return functionOP[i];
			}
		}
	}
	return OP_NULL;
}

void resetValues(double* printVal) {
	// Resets values and arrays between main loops
	for (int i = EVAL_VARS_START; i < VAR_MAP_SIZE; i++) {
		variableMap[i] = 0.0;
	}
	for (int i = 0; i < RPN_SIZE; i++) {
		expressionRPN[i] = 0;
	}
	for (int i = 0; i < INPUT_SIZE; i++) {
		terminalInput[i] = 0;
	}
	for (int i = 0; i < INPUT_HOLDER_SIZE; i++) {
		unrecognizedToken[i] = 0;
	}
	*printVal = 0.0;
	error = NO_ERROR;
}