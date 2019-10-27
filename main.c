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
#define OP_LOG 9
#define OP_ROOT 10
#define OP_SIN 11
#define OP_COS 12
#define OP_TAN 13
#define OP_SEC 14
#define OP_CSC 15
#define OP_COT 16
#define OP_ASIN 17
#define OP_ACOS 18
#define OP_ATAN 19
#define OP_ASEC 20
#define OP_ACSC 21
#define OP_ACOT 22
#define OP_SINH 23
#define OP_COSH 24
#define OP_TANH 25
#define OP_SECH 26
#define OP_CSCH 27
#define OP_COTH 28
#define OP_ASINH 29
#define OP_ACOSH 30
#define OP_ATANH 31
#define OP_ASECH 32
#define OP_ACSCH 33
#define OP_ACOTH 34
#define OP_SQRT 35
#define OP_LN 36
#define OP_LOGT 37 
#define OP_CEIL 38
#define OP_FLOOR 39
#define OP_ROUND 40
#define OP_SIGN 41
#define OP_GCD 42
#define OP_LCM 43
#define OP_ATANT 44 
#define OP_ABS 45 
#define OP_TERMINATE 49

#define NR_FUNCTIONS 39
#define ARG_SEPARATOR 50
#define LEFT_PARENTH 51
#define RIGHT_PARENTH 52
#define VAR_BEGIN 53

#define NO_ERROR 0
#define ERR_SYNTAX 1
#define ERR_OVERFLOW 2
#define ERR_UNKNOWN_TOKEN 3

#define INPUT_SIZE 1024
#define OUTPUT_SIZE 512
#define VAR_VAL_SIZE 128
#define STACK_SIZE 256
#define UNRECOGNIZED_TOKEN_SIZE 5


void resetValues(double variableValues[], char output[], char rawInput[], char unrecognizedToken[], double* printVal, int* error);
char pop(char arr[], int* length);
void push(char arr[], char val, int* length, int maxLength, int* error);
char stackIsEmpty(char stack[]);
bool isFunction(char token);
bool isOperator(char token);
void pushOperator(char token, char stack[], int* stackLength, char output[], int* outputLength, int* error);
char findFunction(char input[], int* error, char unrecognizedToken[]);
char tokenize(char rawInput[], int* indexPtr, double variableValues[], int* varValIndex, bool unaryNegation, int* error, char unrecognizedToken[]);
void inputToRPN(char rawInput[], char output[], double variableValues[], int* error, char unrecognizedToken[]);
double evaluateRPN(char input[], double variableValues[], int* error);

int main() {
	// User enters an expression as input.  It is evaluated and the result is returned

	char rawInput[INPUT_SIZE] = { 0 };
	bool quit = false;
	int error = 0;
	char output[OUTPUT_SIZE] = { 0 };
	double variableValues[VAR_VAL_SIZE] = { 0.0 };
	double printVal = 0.0;
	bool scientificNotation = false;
	char unrecognizedToken[5] = { 0 };

	printf("> ");
	while (fgets(rawInput, INPUT_SIZE, stdin) && !quit) {
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
			printf("  unrecognized token: ");
			for (int i = 0; i < UNRECOGNIZED_TOKEN_SIZE; i++){
				printf("%c", unrecognizedToken[i]);
			}
			printf("\n");
		}
		else { // overflow
			printf("overflow error\n");
		}

		printf("\n> ");
		resetValues(variableValues, output, rawInput, unrecognizedToken, &printVal, &error);
	}

	return 0;
}

void resetValues(double variableValues[], char output[], char rawInput[], char unrecognizedToken[], double* printVal, int* error) {
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

char pop(char arr[], int* length) {
	// Removes LAST element from array, returns that element, and decrements array length
	if (*length > 0) {
		char val = arr[*length - 1];
		arr[*length - 1] = 0;
		(*length)--;
		return val;
	}
	return OP_NULL;
}

void push(char arr[], char val, int* length, int maxLength, int* error) {
	// Appends input to the end of array, increments array length
	if (*length >= maxLength) {
		*error = ERR_OVERFLOW;
		return;
	}
	arr[*length] = val;
	(*length)++;
}

char stackIsEmpty(char stack[]) {
	// Returns true if given stack array is empty
	return (stack[0] == 0);
}

bool isFunction(char token) {
	// Returns true if given operator uses function notation, func(arg1, arg2)
	return (token > OP_EXP && token < OP_TERMINATE);
}

bool isOperator(char token) {
	// Returns true if given token is an operator
	return (token >= 0 && token < LEFT_PARENTH);
}

void pushOperator(char token, char stack[], int* stackLength, char output[], int* outputLength, int* error) {
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
		if (*error != 0) return;
	}
	else {
		// Push all higher precedence tokens from stack to output, then push current token to stack
		while (!stackIsEmpty(stack)) {
			if (isFunction(stack[*stackLength - 1])) {
				// Functions, being in prefix rather than infix notation, are simply pushed
				push(output, pop(stack, stackLength), outputLength, OUTPUT_SIZE, error);
				if (*error != 0) return;
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

char findFunction(char input[], int* error, char unrecognizedToken[]) { 
	// Returns the op-code for a given string representing a function. Maximum length is 5. Unused (terminal) characters must be null

	char functions[NR_FUNCTIONS][5] = { "idiv\0", "mod\0\0", "log\0\0", "root\0", 
		"sin\0\0", "cos\0\0", "tan\0\0", "sec\0\0", "csc\0\0", "cot\0\0", 
		"asin\0", "acos\0", "atan\0", "asec\0", "acsc\0", "acot\0",
		"sinh\0", "cosh\0", "tanh\0", "sech\0", "csch\0", "coth\0",
		"asinh", "acosh", "atanh", "asech", "acsch", "acoth",
		"sqrt\0", "ln\0\0\0", "logt\0", "ceil\0", "floor", "round", "sign\0", "gcd\0\0", "lcm\0\0", "atant", "abs\0\0"};
	int functionOP[NR_FUNCTIONS] = { OP_DIV_INT, OP_MOD, OP_LOG, OP_ROOT, 
		OP_SIN, OP_COS, OP_TAN, OP_SEC, OP_CSC, OP_COT, 
		OP_ASIN, OP_ACOS, OP_ATAN, OP_ASEC, OP_ACSC, OP_ACOT, 
		OP_SINH, OP_COSH, OP_TANH, OP_SECH, OP_CSCH, OP_COTH, 
		OP_ASINH, OP_ACOSH, OP_ATANH, OP_ASECH, OP_ACSCH, OP_ACOTH, 
		OP_SQRT, OP_LN, OP_LOGT, OP_CEIL, OP_FLOOR, OP_ROUND, OP_SIGN, OP_GCD, OP_LCM, OP_ATANT, OP_ABS};

	for (int i = 0; i < NR_FUNCTIONS; i++) { // function loop
		for (int j = 0; j < 5; j++) { // character loop
			if (input[j] != functions[i][j]) { 
				// If at any point the input and current function don't match, try next function
				break;
			}
			else if (j == 4) {
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

char tokenize(char rawInput[], int* indexPtr, double variableValues[], int* varValIndex, bool unaryNegation, int* error, char unrecognizedToken[]) {
	// Converts multi-character inputs (such as function names) into their representative tokens

	char currChar = rawInput[*indexPtr];
	char inputHolder[5] = { 0 };
	int inputHolderIndex = 0;
	int numDecimalPlaces = 0; // If this exeeds one for a number, error
	char outputToken = 0;

	if ((currChar >= '0' && currChar <= '9') || currChar == '.') { 
		// If token is a value (currently does not support scientific notation)
		while ((currChar >= '0' && currChar <= '9') || currChar == '.') { 
			// Scan all characters until no longer part of a value
			if (currChar == '.') numDecimalPlaces++;
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
			if (inputHolderIndex >= 5) {
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

void inputToRPN(char rawInput[], char output[], double variableValues[], int* error, char unrecognizedToken[]) {
	// Converts infix input to array of postfix tokens

	char stack[STACK_SIZE] = { 0 }; int stackLength = 0;
	int outputLength = 0;
	char token = 0;
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
	push(output, OP_TERMINATE, &outputLength, OUTPUT_SIZE, error);
}

double evaluateRPN(char input[], double variableValues[], int* error) {
	char stack[STACK_SIZE] = { 0 }; int stackLength = 0;
	int valueIndexLeft = 0; int valueIndexRight = 0;
	char operand = 0;
	bool binary;

	for (int inputIndex = 0; inputIndex < 128; inputIndex++) {
		if (input[inputIndex] == OP_TERMINATE) {
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
				if (!(operand >= OP_SIN && operand <= OP_ACOTH) && !(operand == OP_NEG)) {
					// Binary operators will act on another value
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
				case OP_DIV_INT:
					variableValues[valueIndexLeft] = (double)(round(variableValues[valueIndexLeft]) / round(variableValues[valueIndexRight]));
					break;
				case OP_MOD:
					variableValues[valueIndexLeft] = (double)((int)(round(variableValues[valueIndexLeft]) + 0.01) 
						% (int)(round(variableValues[valueIndexRight]) + 0.01));
					break;
				case OP_EXP:
					variableValues[valueIndexLeft] = pow(variableValues[valueIndexLeft], variableValues[valueIndexRight]);
					break;
				case OP_LOG:
					variableValues[valueIndexLeft] = log10(variableValues[valueIndexRight]) / log10(variableValues[valueIndexLeft]);
					break;
				case OP_ROOT:
					variableValues[valueIndexLeft] = pow(variableValues[valueIndexRight], (1 / variableValues[valueIndexLeft]));
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
				default:
					break;
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