/*
Created by Juliusz Jakobowski

A command line program that recieves a mathematical expression as input, evaluates the expression, and returns the result.
Implemented using a shunting yard algorithm modified to accept functions, implicit multiplication, and ability to distinguish
unary negation and binary subtraction.

Released under the MIT License.
*/

#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include<stdbool.h>

#define OP_NULL 0
#define OP_ADD 1
#define OP_SUB 2
#define OP_NEG 3
#define OP_MUL 4
#define OP_DIV 5
#define OP_EXP 6
#define OP_DIV_INT 7
#define OP_MOD 8
#define OP_GCD 9
#define OP_LCM 10
#define OP_CEIL 11
#define OP_FLOOR 12
#define OP_ROUND 13
#define OP_TRUNC 14
#define OP_SIGN 15
#define OP_ABS 16 
#define OP_LOG 17
#define OP_LN 18
#define OP_LOGT 19 
#define OP_ROOT 20
#define OP_SQRT 21
#define OP_CBRT 22
#define OP_HYPOT 23
#define OP_SIN 24
#define OP_COS 25
#define OP_TAN 26
#define OP_SEC 27
#define OP_CSC 28
#define OP_COT 29
#define OP_ASIN 30
#define OP_ACOS 31
#define OP_ATAN 32
#define OP_ASEC 33
#define OP_ACSC 34
#define OP_ACOT 35
#define OP_SINH 36
#define OP_COSH 37
#define OP_TANH 38
#define OP_SECH 39
#define OP_CSCH 40
#define OP_COTH 41
#define OP_ASINH 42
#define OP_ACOSH 43
#define OP_ATANH 44
#define OP_ASECH 45
#define OP_ACSCH 46
#define OP_ACOTH 47
#define OP_ATANT 48
#define OP_SINC 49
#define OP_NSINC 50
#define OP_ERF 51
#define OP_ERFC 52
#define OP_GAMMA 53
#define OP_LGAMM 54
#define OP_REQLL 55

#define ARG_SEPARATOR 125
#define LEFT_PARENTH 126
#define RIGHT_PARENTH 127
#define VAR_BEGIN 128

#define NO_ERROR 0
#define ERR_SYNTAX 1
#define ERR_OVERFLOW 2
#define ERR_UNKNOWN_TOKEN 3

#define NR_FUNCTIONS 49
#define INPUT_HOLDER_SIZE 32
#define INPUT_SIZE 1024
#define OUTPUT_SIZE 512
#define VAR_VAL_SIZE 128
#define STACK_SIZE 256
#define UNRECOGNIZED_TOKEN_SIZE 6

#define pi 3.14159265358979323846


void resetValues(double variableValues[], unsigned char output[], char rawInput[], char unrecognizedToken[], double* printVal, int* error);
unsigned char pop(unsigned char arr[], int* length);
void push(unsigned char arr[], unsigned char val, int* length, int maxLength, int* error);
bool stackIsEmpty(unsigned char stack[]);
bool isFunction(unsigned char token);
bool isOperator(unsigned char token);
bool isBinaryOperator(unsigned char token);
void pushOperator(unsigned char token, unsigned char stack[], int* stackLength, unsigned char output[], int* outputLength, int* error);
long long int doubleToInt(double input);
double gcd(double a, double b);
unsigned char findFunction(char input[], int* error, char unrecognizedToken[]);
unsigned char tokenize(char rawInput[], int* indexPtr, double variableValues[], int* varValIndex, bool unaryNegation, int* error, char unrecognizedToken[]);
void inputToRPN(char rawInput[], unsigned char output[], double variableValues[], int* error, char unrecognizedToken[]);
double evaluateRPN(unsigned char input[], double variableValues[], int* error);

int main() {
	// User enters an expression as input.  It is evaluated and the result is returned, barring any errors

	char rawInput[INPUT_SIZE] = { 0 };
	bool quit = false;
	int error = 0;
	unsigned char output[OUTPUT_SIZE] = { 0 };
	double variableValues[VAR_VAL_SIZE] = { 0.0 };
	double printVal = 0.0;
	bool scientificNotation = false;
	char unrecognizedToken[UNRECOGNIZED_TOKEN_SIZE] = { 0 };

	printf("> ");
	while (fgets(rawInput, INPUT_SIZE, stdin) && !quit) {
		const int UNRECOGNIZED_TOKEN_LAST_ELEMENT = UNRECOGNIZED_TOKEN_SIZE - 1;

		if (rawInput[INPUT_SIZE - 2] != '\0') {
			error = ERR_OVERFLOW;
		}

		// Check for commands
		if (rawInput[0] == 'q' && rawInput[1] == 'u' && rawInput[2] == 'i' && rawInput[3] == 't') {
			break;
		}

		// Perform calculation
		if (error == NO_ERROR) {
			inputToRPN(rawInput, output, variableValues, &error, unrecognizedToken);
		}
		if (error == NO_ERROR) {
			printVal = evaluateRPN(output, variableValues, &error);
		}

		// Error checking and printing
		if (error == NO_ERROR) {
			printf("  %lf\n", printVal);
		}
		else if (error == ERR_SYNTAX) {
			printf("  syntax error\n");
		}
		else if (error == ERR_UNKNOWN_TOKEN) {
			printf("  unrecognized token \"");
			for (int i = 0; i < UNRECOGNIZED_TOKEN_LAST_ELEMENT; i++) {
				if (unrecognizedToken[i] == '\0') break;
				printf("%c", unrecognizedToken[i]);
			}

			if (unrecognizedToken[UNRECOGNIZED_TOKEN_LAST_ELEMENT] != 0) printf("...");
			printf("\"\n");
		}
		else { // overflow
			printf("  overflow error\n");
		}

		printf("\n> ");
		resetValues(variableValues, output, rawInput, unrecognizedToken, &printVal, &error);
	}

	return 0;
}

void resetValues(double variableValues[], unsigned char output[], char rawInput[], char unrecognizedToken[], double* printVal, int* error) {
	// Resets values and arrays between main loops
	for (int i = 0; i < VAR_VAL_SIZE; i++) {
		variableValues[i] = 0.0;
	}
	for (int i = 0; i < OUTPUT_SIZE; i++) {
		output[i] = 0;
	}
	for (int i = 0; i < INPUT_SIZE; i++) {
		rawInput[i] = 0;
	}
	for (int i = 0; i < UNRECOGNIZED_TOKEN_SIZE; i++) {
		unrecognizedToken[i] = 0;
	}
	*printVal = 0.0;
	*error = 0;
}

unsigned char pop(unsigned char arr[], int* length) {
	// Removes LAST element from array, returns that element, and decrements array length
	if (*length > 0) {
		unsigned char val = arr[*length - 1];
		arr[*length - 1] = 0;
		(*length)--;
		return val;
	}
	return OP_NULL;
}

void push(unsigned char arr[], unsigned char val, int* length, int maxLength, int* error) {
	// Appends input to the end of array, increments array length
	if (*length >= maxLength) {
		*error = ERR_OVERFLOW;
		return;
	}
	arr[*length] = val;
	(*length)++;
}

bool stackIsEmpty(unsigned char stack[]) {
	// Returns true if given stack array is empty
	return (stack[0] == 0);
}

bool isFunction(unsigned char token) {
	// Returns true if given operator uses function notation, func(arg1, arg2)
	return (token > OP_EXP && token < ARG_SEPARATOR);
}

bool isOperator(unsigned char token) {
	// Returns true if given token is an operator
	return (token >= 0 && token < LEFT_PARENTH);
}

bool isBinaryOperator(unsigned char token) {
	// Returns true if function or operator has two inputs, false if one
	return ((token == OP_ADD || token == OP_SUB)
		|| (token >= OP_MUL && token <= OP_LCM)
		|| (token == OP_LOG) || (token == OP_ROOT)
		|| (token == OP_HYPOT) || (token == OP_ATANT)
		|| (token == OP_REQLL));
}

void pushOperator(unsigned char token, unsigned char stack[], int* stackLength, unsigned char output[], int* outputLength, int* error) {
	// Pushes given token and some tokens on stack to the output in such a manner that output is in postfix

	int precedence[7] = {
		0, //NULL
		1, //ADD
		1, //SUB
		3, //NEG
		2, //MUL
		2, //DIV
		4, //EXP
	};

	int stackPrecedence = 0; int tokenPrecedence = 0;

	if (stackIsEmpty(stack)) {
		push(stack, token, stackLength, STACK_SIZE, error);
		if (*error != NO_ERROR) return;
	}
	else {
		// Push all higher precedence tokens from stack to output, then push current token to stack
		while (!stackIsEmpty(stack)) {
			if (isFunction(stack[*stackLength - 1])) {
				// Functions, being in prefix rather than infix notation, are simply pushed
				push(output, pop(stack, stackLength), outputLength, OUTPUT_SIZE, error);
				if (*error != NO_ERROR) return;
			}
			else {
				stackPrecedence = precedence[stack[*stackLength - 1]];
				tokenPrecedence = precedence[token];

				if ((token != OP_EXP && tokenPrecedence <= stackPrecedence)
					|| (token == OP_EXP && tokenPrecedence < stackPrecedence)) {
					push(output, pop(stack, stackLength), outputLength, OUTPUT_SIZE, error);
					if (*error != NO_ERROR) return;
				}
				else break;
			}
		}
		push(stack, token, stackLength, STACK_SIZE, error);
	}
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

unsigned char findFunction(char input[], int* error, char unrecognizedToken[]) {
	// Returns the op-code for a given string representing a function. Maximum length is 5. Unused (terminal) characters must be null

	char functions[NR_FUNCTIONS][5] = { "idiv\0", "mod\0\0", "log\0\0", "root\0",
		"sin\0\0", "cos\0\0", "tan\0\0", "sec\0\0", "csc\0\0", "cot\0\0",
		"asin\0", "acos\0", "atan\0", "asec\0", "acsc\0", "acot\0",
		"sinh\0", "cosh\0", "tanh\0", "sech\0", "csch\0", "coth\0",
		"asinh", "acosh", "atanh", "asech", "acsch", "acoth",
		"sqrt\0", "ln\0\0\0", "logt\0", "ceil\0", "floor", "round", "sgn\0\0", "gcd\0\0", "lcm\0\0", "atant", "abs\0\0",
		"cbrt\0", "trunc", "erf\0\0", "erfc\0", "gamma", "hypot", "lgamm", "sinc\0", "nsinc", "reqll" };
	int functionOP[NR_FUNCTIONS] = { OP_DIV_INT, OP_MOD, OP_LOG, OP_ROOT,
		OP_SIN, OP_COS, OP_TAN, OP_SEC, OP_CSC, OP_COT,
		OP_ASIN, OP_ACOS, OP_ATAN, OP_ASEC, OP_ACSC, OP_ACOT,
		OP_SINH, OP_COSH, OP_TANH, OP_SECH, OP_CSCH, OP_COTH,
		OP_ASINH, OP_ACOSH, OP_ATANH, OP_ASECH, OP_ACSCH, OP_ACOTH,
		OP_SQRT, OP_LN, OP_LOGT, OP_CEIL, OP_FLOOR, OP_ROUND, OP_SIGN, OP_GCD, OP_LCM, OP_ATANT, OP_ABS,
		OP_CBRT, OP_TRUNC, OP_ERF, OP_ERFC, OP_GAMMA, OP_HYPOT, OP_LGAMM, OP_SINC, OP_NSINC, OP_REQLL };

	for (int i = 0; i < NR_FUNCTIONS; i++) { // function loop
		for (int j = 0; j < 5; j++) { // character loop
			if (input[j] != functions[i][j]) {
				// If at any point the input and current function don't match, try next function
				break;
			}
			else if (j == 4 && input[5] == '\0') {
				// Return if end of the input has been reached and all have matched
				return functionOP[i];
			}
		}

	}
	*error = ERR_UNKNOWN_TOKEN;
	for (int i = 0; i < UNRECOGNIZED_TOKEN_SIZE; i++) {
		unrecognizedToken[i] = input[i];
	}
	return OP_NULL;
}

unsigned char tokenize(char rawInput[], int* indexPtr, double variableValues[], int* varValIndex, bool unaryNegation, int* error, char unrecognizedToken[]) {
	// Converts multi-character inputs (such as function names) into their representative tokens

	char currChar = rawInput[*indexPtr];
	char inputHolder[INPUT_HOLDER_SIZE] = { 0 };
	int inputHolderIndex = 0;
	int numDecimalPlaces = 0; // If this exeeds one for a number, error
	unsigned char outputToken = 0;

	if ((currChar >= '0' && currChar <= '9') || currChar == '.') {
		// If token is a value (currently does not support scientific notation)
		while ((currChar >= '0' && currChar <= '9') || currChar == '.') {
			// Scan all characters until no longer part of a value
			if (currChar == '.') numDecimalPlaces++;
			if (inputHolderIndex >= 32) {
				*error = ERR_OVERFLOW;
				break;
			}
			inputHolder[inputHolderIndex] = currChar;
			(*indexPtr)++;
			inputHolderIndex++;
			currChar = rawInput[*indexPtr];
		}

		// Validate number, convert to double and store
		if (numDecimalPlaces > 1) return OP_NULL;
		if (*varValIndex >= VAR_VAL_SIZE) {
			*error = ERR_OVERFLOW;
			return OP_NULL;
		}
		variableValues[*varValIndex] = atof(inputHolder);
		outputToken = *varValIndex + VAR_BEGIN;
		(*varValIndex)++;
	}
	else if (currChar >= 'a' && currChar <= 'z') {
		// If token is a function
		while (currChar >= 'a' && currChar <= 'z') {
			if (inputHolderIndex >= 6) {
				break;
			}
			// Scan characters until no longer can be part of a function
			inputHolder[inputHolderIndex] = currChar;
			(*indexPtr)++;
			inputHolderIndex++;
			currChar = rawInput[*indexPtr];
		}

		outputToken = findFunction(inputHolder, error, unrecognizedToken);
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
		case '-':
			outputToken = (unaryNegation) ? OP_NEG : OP_SUB;
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
		default:
			outputToken = OP_NULL;
			*error = ERR_UNKNOWN_TOKEN;
			unrecognizedToken[0] = currChar;
			break;
		}
		(*indexPtr)++;
	}

	return outputToken;
}

void inputToRPN(char rawInput[], unsigned char output[], double variableValues[], int* error, char unrecognizedToken[]) {
	// Converts infix input to array of postfix tokens

	unsigned char stack[STACK_SIZE] = { 0 }; int stackLength = 0;
	int outputLength = 0;
	unsigned char token = 0;
	int varValIndex = 0;

	bool implicitMultiplication = false;
	// Whether or not the next token has the ability to be implicitly multiplied, such as with parentheses: 3(5) = 15

	bool unaryNegation = true;
	// Whether the next '-' encountered should be interpreted as negation (true) or subtraction (false)

	int index = 0;

	while (index < INPUT_SIZE) {
		if (rawInput[index] == '\n') {
			// If end of input has been reached
			break;
		}
		token = tokenize(rawInput, &index, variableValues, &varValIndex, unaryNegation, error, unrecognizedToken);
		if (*error != NO_ERROR) {
			return;
		}

		if (token == OP_NULL) {
			continue;
		}
		else if (token == ARG_SEPARATOR) {
			// Comma that separates function arguments.  Operators are popped until left parentheses encountered
			while (stack[stackLength - 1] != LEFT_PARENTH && !stackIsEmpty(stack)) {
				push(output, pop(stack, &stackLength), &outputLength, OUTPUT_SIZE, error);
				if (*error != 0) return;
			}
			unaryNegation = true;
			implicitMultiplication = false;
		}
		else if (token == OP_NEG) {
			// Negation may be applied to number to its right directly after an operator, so cannot pop any operators from stack
			push(stack, token, &stackLength, STACK_SIZE, error);
		}
		else if (isFunction(token)) {
			if (implicitMultiplication) {
				pushOperator(OP_MUL, stack, &stackLength, output, &outputLength, error);
			}
			push(stack, token, &stackLength, STACK_SIZE, error);
			implicitMultiplication = false;
			unaryNegation = true;
		}
		else if (isOperator(token)) {
			// Non-function operators
			pushOperator(token, stack, &stackLength, output, &outputLength, error);
			implicitMultiplication = false;
			unaryNegation = true;
		}
		else if (token == LEFT_PARENTH) {
			if (implicitMultiplication) {
				pushOperator(OP_MUL, stack, &stackLength, output, &outputLength, error);
			}
			push(stack, LEFT_PARENTH, &stackLength, STACK_SIZE, error);
			implicitMultiplication = false;
			unaryNegation = true;
		}
		else if (token == RIGHT_PARENTH) {
			// Pop operators until left parentheses encountered, then pop the left parenthesis
			if (!stackIsEmpty(stack)) {
				while (stack[stackLength - 1] != LEFT_PARENTH && !stackIsEmpty(stack)) {
					push(output, pop(stack, &stackLength), &outputLength, OUTPUT_SIZE, error);
					if (*error != NO_ERROR) return;
				}
				pop(stack, &stackLength);
			}
		}
		else {
			// If token is a variable
			if (implicitMultiplication) {
				pushOperator(OP_MUL, stack, &stackLength, output, &outputLength, error);
			}
			push(output, token, &outputLength, OUTPUT_SIZE, error);
			implicitMultiplication = true;
			unaryNegation = false;
		}
	}

	while (!(stack[0] == 0)) {
		// Push all remaining operators from stack
		push(output, pop(stack, &stackLength), &outputLength, OUTPUT_SIZE, error);
		if (*error != NO_ERROR) return;
	}
}

double evaluateRPN(unsigned char input[], double variableValues[], int* error) {
	// Given input in RPN format and a list of variable values, performs the actual calculations of the program in the given order and returns the final result

	unsigned char stack[STACK_SIZE] = { 0 }; int stackLength = 0;
	int valueIndexLeft = 0; int valueIndexRight = 0;
	unsigned char operand = 0;
	bool binary;

	for (int inputIndex = 0; inputIndex < OUTPUT_SIZE; inputIndex++) {
		if (input[inputIndex] == 0) {
			break;
		}
		binary = false;

		// If current input is variable or there is no operator on the stack, the input is pushed
		if (input[inputIndex] > RIGHT_PARENTH || stackIsEmpty(stack) || stack[stackLength - 1] < LEFT_PARENTH) {
			push(stack, input[inputIndex], &stackLength, STACK_SIZE, error);
			if (*error != NO_ERROR) return 0.0;
		}
		// Otherwise, the operation atop the stack is executed using the variables directly below it; lower variable recieves the result
		else {
			push(stack, input[inputIndex], &stackLength, STACK_SIZE, error);
			if (*error != NO_ERROR) return 0.0;
			if (stackLength >= 2) {
				operand = stack[stackLength - 1];
				valueIndexRight = stack[stackLength - 2] - VAR_BEGIN;
				if (isBinaryOperator(operand)) {
					// Binary operators will act on two values
					if (stackLength >= 3) {
						valueIndexLeft = stack[stackLength - 3] - VAR_BEGIN;
						binary = true;
					}
					else {
						*error = ERR_SYNTAX;
						return 0.0;
					}
				}
				switch (operand) {
				case OP_ADD:
					variableValues[valueIndexLeft] = variableValues[valueIndexLeft] + variableValues[valueIndexRight];
					break;
				case OP_SUB:
					variableValues[valueIndexLeft] = variableValues[valueIndexLeft] - variableValues[valueIndexRight];
					break;
				case OP_NEG:
					variableValues[valueIndexRight] = -(variableValues[valueIndexRight]);
					break;
				case OP_MUL:
					variableValues[valueIndexLeft] = variableValues[valueIndexLeft] * variableValues[valueIndexRight];
					break;
				case OP_DIV:
					variableValues[valueIndexLeft] = variableValues[valueIndexLeft] / variableValues[valueIndexRight];
					break;
				case OP_EXP:
					variableValues[valueIndexLeft] = pow(variableValues[valueIndexLeft], variableValues[valueIndexRight]);
					break;
				case OP_DIV_INT:
					variableValues[valueIndexLeft] = (double)(doubleToInt(variableValues[valueIndexLeft]) / doubleToInt(variableValues[valueIndexRight]));
					break;
				case OP_MOD:
					variableValues[valueIndexLeft] = (double)((int)(round(variableValues[valueIndexLeft]) + 0.01)
						% (int)(round(variableValues[valueIndexRight]) + 0.01));
					break;
				case OP_GCD:
					variableValues[valueIndexLeft] = gcd(variableValues[valueIndexRight], variableValues[valueIndexLeft]);
					break;
				case OP_LCM:
					variableValues[valueIndexLeft] = (variableValues[valueIndexRight] / 
									gcd(variableValues[valueIndexRight], variableValues[valueIndexLeft])) * variableValues[valueIndexLeft];
					break;
				case OP_CEIL:
					variableValues[valueIndexRight] = ceil(variableValues[valueIndexRight]);
					break;
				case OP_FLOOR:
					variableValues[valueIndexRight] = floor(variableValues[valueIndexRight]);
					break;
				case OP_ROUND:
					variableValues[valueIndexRight] = round(variableValues[valueIndexRight]);
					break;
				case OP_TRUNC:
					variableValues[valueIndexRight] = trunc(variableValues[valueIndexRight]);
					break;
				case OP_SIGN:
					if (variableValues[valueIndexRight] >= 0.0) {
						variableValues[valueIndexRight] = 1.0;
					}
					else {
						variableValues[valueIndexRight] = -1.0;
					}
					break;
				case OP_ABS:
					variableValues[valueIndexRight] = fabs(variableValues[valueIndexRight]);
					break;
				case OP_LOG:
					variableValues[valueIndexLeft] = log10(variableValues[valueIndexRight]) / log10(variableValues[valueIndexLeft]);
					break;
				case OP_LN:
					variableValues[valueIndexRight] = log(variableValues[valueIndexRight]);
					break;
				case OP_LOGT:
					variableValues[valueIndexRight] = log10(variableValues[valueIndexRight]);
					break;
				case OP_ROOT:
					variableValues[valueIndexLeft] = pow(variableValues[valueIndexRight], (1 / variableValues[valueIndexLeft]));
					break;
				case OP_SQRT:
					variableValues[valueIndexRight] = sqrt(variableValues[valueIndexRight]);
					break;
				case OP_CBRT:
					variableValues[valueIndexRight] = cbrt(variableValues[valueIndexRight]);
					break;
				case OP_HYPOT:
					variableValues[valueIndexLeft] = hypot(variableValues[valueIndexLeft], variableValues[valueIndexRight]);
					break;
				case OP_SIN:
					variableValues[valueIndexRight] = sin(variableValues[valueIndexRight]);
					break;
				case OP_COS:
					variableValues[valueIndexRight] = cos(variableValues[valueIndexRight]);
					break;
				case OP_TAN:
					variableValues[valueIndexRight] = tan(variableValues[valueIndexRight]);
					break;
				case OP_SEC:
					variableValues[valueIndexRight] = 1 / cos(variableValues[valueIndexRight]);
					break;
				case OP_CSC:
					variableValues[valueIndexRight] = 1 / sin(variableValues[valueIndexRight]);
					break;
				case OP_COT:
					variableValues[valueIndexRight] = 1 / tan(variableValues[valueIndexRight]);
					break;
				case OP_ASIN:
					variableValues[valueIndexRight] = asin(variableValues[valueIndexRight]);
					break;
				case OP_ACOS:
					variableValues[valueIndexRight] = acos(variableValues[valueIndexRight]);
					break;
				case OP_ATAN:
					variableValues[valueIndexRight] = atan(variableValues[valueIndexRight]);
					break;
				case OP_ASEC:
					variableValues[valueIndexRight] = acos(1 / (variableValues[valueIndexRight]));
					break;
				case OP_ACSC:
					variableValues[valueIndexRight] = asin(1 / (variableValues[valueIndexRight]));
					break;
				case OP_ACOT:
					if (variableValues[valueIndexRight] > 0) {
						variableValues[valueIndexRight] = atan(1 / variableValues[valueIndexRight]);
					}
					else if (variableValues[valueIndexRight] < 0) {
						variableValues[valueIndexRight] = atan(1 / variableValues[valueIndexRight]) + pi;
					}
					else {
						variableValues[valueIndexRight] = pi / 2;
					}
					break;
				case OP_SINH:
					variableValues[valueIndexRight] = sinh(variableValues[valueIndexRight]);
					break;
				case OP_COSH:
					variableValues[valueIndexRight] = cosh(variableValues[valueIndexRight]);
					break;
				case OP_TANH:
					variableValues[valueIndexRight] = tanh(variableValues[valueIndexRight]);
					break;
				case OP_SECH:
					variableValues[valueIndexRight] = 1 / cosh(variableValues[valueIndexRight]);
					break;
				case OP_CSCH:
					variableValues[valueIndexRight] = 1 / sinh(variableValues[valueIndexRight]);
					break;
				case OP_COTH:
					variableValues[valueIndexRight] = 1 / tanh(variableValues[valueIndexRight]);
					break;
				case OP_ASINH:
					variableValues[valueIndexRight] = asinh(variableValues[valueIndexRight]);
					break;
				case OP_ACOSH:
					variableValues[valueIndexRight] = acosh(variableValues[valueIndexRight]);
					break;
				case OP_ATANH:
					variableValues[valueIndexRight] = atanh(variableValues[valueIndexRight]);
					break;
				case OP_ASECH:
					variableValues[valueIndexRight] = acosh(1 / (variableValues[valueIndexRight]));
					break;
				case OP_ACSCH:
					variableValues[valueIndexRight] = asinh(1 / (variableValues[valueIndexRight]));
					break;
				case OP_ACOTH:
					variableValues[valueIndexRight] = atanh(1 / (variableValues[valueIndexRight]));
					break;
				case OP_ATANT:
					variableValues[valueIndexLeft] = atan2(variableValues[valueIndexLeft], variableValues[valueIndexRight]);
					break;
				case OP_SINC:
					if (variableValues[valueIndexRight] == 0.0) variableValues[valueIndexRight] = 1.0;
					else variableValues[valueIndexRight] = sin(variableValues[valueIndexRight]) / variableValues[valueIndexRight];
					break;
				case OP_NSINC:
					if (variableValues[valueIndexRight] == 0.0) variableValues[valueIndexRight] = 1.0;
					else variableValues[valueIndexRight] = sin(pi * variableValues[valueIndexRight]) / (pi * variableValues[valueIndexRight]);
					break;
				case OP_ERF:
					variableValues[valueIndexRight] = erf(variableValues[valueIndexRight]);
					break;
				case OP_ERFC:
					variableValues[valueIndexRight] = erfc(variableValues[valueIndexRight]);
					break;
				case OP_GAMMA:
					variableValues[valueIndexRight] = tgamma(variableValues[valueIndexRight]);
					break;
				case OP_LGAMM:
					variableValues[valueIndexRight] = lgamma(variableValues[valueIndexRight]);
					break;
				case OP_REQLL:
					variableValues[valueIndexLeft] = (variableValues[valueIndexLeft] * variableValues[valueIndexRight]) 
													/ (variableValues[valueIndexLeft] + variableValues[valueIndexRight]);
					break;
				default:
					*error = ERR_SYNTAX;
					return 0.0;
				}

				pop(stack, &stackLength);
				if (binary) {
					pop(stack, &stackLength);
				}
			}
			else {
				*error = ERR_SYNTAX;
				return 0.0;
			}
		}
	}

	if (stack[0] < VAR_BEGIN || stack[1] != 0) {
		// If a function is left on the stack, there were too many operators
		// If there is more than one thing left on the stack, an error occurred (most likely a misplaced argument separator)
		*error = ERR_SYNTAX;
	}
	return variableValues[0];
}
