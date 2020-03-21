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

typedef enum OPERATORS {
//============= BINARY (expept for NEG and NOT) =============//
/* basic        */ OP_NULL, OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD, OP_NEG,
/* equality     */ OP_IS, OP_GREATER_THAN, OP_LESS_THAN, OP_GREATER_THAN_EQUAL_TO, OP_LESS_THAN_EQUAL_TO,
/* logic        */ OP_AND, OP_OR, OP_NOT, OP_XOR, OP_IMPLIES, OP_IFF, OP_IMPLIED_BY,
/* bitwise      */ OP_RIGHT_SHIFT, OP_LEFT_SHIFT, OP_BITWISE_AND, OP_BITWISE_OR, OP_BITWISE_NOT, OP_BITWISE_XOR,
/* powers       */ OP_EXP, END_OPS = OP_EXP, OP_LOG, OP_ROOT,
/* discrete     */ OP_DIV_INT, OP_GCD, OP_LCM, OP_NCR, OP_NPR,
/* trig-related */ OP_ATAN2,
/* misc         */ OP_HYPOT, OP_REQLL, OP_PERR,
//========================== UNARY ==========================//
/* powers       */ OP_LOG2, UNARY_OPERATORS = OP_LOG2, OP_LOG10, OP_LN, OP_SQRT, OP_CBRT,
/* trig         */ OP_SIN, OP_COS, OP_TAN, OP_SEC, OP_CSC, OP_COT, OP_ASIN, OP_ACOS, OP_ATAN, OP_ASEC, OP_ACSC, OP_ACOT,
/* hyperbolic   */ OP_SINH, OP_COSH, OP_TANH, OP_SECH, OP_CSCH, OP_COTH, OP_ASINH, OP_ACOSH, OP_ATANH, OP_ASECH, OP_ACSCH, OP_ACOTH,
/* discrete     */ OP_CEIL, OP_FLOOR, OP_ROUND, OP_TRUNC, OP_SIGN, OP_ABS,
/* trig-related */ OP_SINC, OP_NSINC, OP_DEG, OP_RAD,
/* misc         */ OP_ERF, OP_ERFC, OP_GAMMA, OP_LGAMMA,

//========================== OTHER ==========================//
/* instructions */ INST_ASSIGN_VAL, END_FUNCS = INST_ASSIGN_VAL, INST_JUMP, INST_JUMP_IF_FALSE,
/* flow         */ ARG_SEPARATOR, LEFT_PARENTH, RIGHT_PARENTH, VAR_BEGIN
};

typedef enum ERRORS { NO_ERROR, ERR_SYNTAX, ERR_OVERFLOW, ERR_UNKNOWN_TOKEN, ERR_UNDEFINED, ERR_OUT_OF_BOUNDS_ANSWER };

typedef enum COMMANDS { CMD_NULL , CMD_QUIT, CMD_SCI, CMD_DEC, CMD_LS };


#define OUTPUT_DECIMAL 0
#define OUTPUT_SCIENTIFIC 1

#define NR_FUNCTIONS 59
#define INPUT_HOLDER_SIZE 32
#define INPUT_SIZE 1024
#define OUTPUT_SIZE 512
#define VAR_VAL_SIZE 128
#define STACK_SIZE 256
#define VARIABLE_SIZE 256
#define SESSION_VAR_START 64
#define LOAD_VAR_HOLDER_SIZE 128
#define FILENAME_SIZE 64

#define pi               3.14159265358979323846
#define RAD_TO_DEG_CONST 57.29577951308232286465
#define DEG_TO_RAD_CONST 0.01745329251994329547

typedef struct {
	double* data;
	int* size;
	int dimensions;
} array;

void resetValues(double variableValues[], unsigned char output[], char rawInput[], char unrecognizedToken[],
				 double* printVal, int* error, int* numDecimals);
int findCommand(char input[]);
void loadVariables(char userVars[][10], double userVarVals[], int VAR_START_POSITION, int VAR_END_POSITION, char filename[64]);
void saveVariable(char userVars[][10], double userVarVals[], int VAR_START_POSITION, int VAR_END_POSITION, char filename[]);
int findNumDecimals(double input);
void inputToRPN(char rawInput[], unsigned char output[], double variableValues[], int* error,
				char unrecognizedToken[], char userVars[][10], double userVarVals[]);
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
	char unrecognizedToken[INPUT_HOLDER_SIZE] = { 0 };
	int numDecimals = 0;
	char userVars[VARIABLE_SIZE][10] = { '\0' }; userVars[0][0] = 'a';  userVars[0][1] = 'n'; userVars[0][2] = 's';
	double userVarVals[VARIABLE_SIZE] = { 0.0 };
	int numSpacesOffset = 0;
	char filename[64] = { '\0' };
	int command = 0;

	loadVariables(userVars, userVarVals, 1, SESSION_VAR_START, "defaultvars.txt");

	printf("> ");
	while (fgets(rawInput, INPUT_SIZE, stdin) && !quit) {
		const int UNRECOGNIZED_TOKEN_LAST_ELEMENT = INPUT_HOLDER_SIZE - 1;

		if (rawInput[INPUT_SIZE - 2] != '\0') {
			error = ERR_OVERFLOW;
		}

		command = findCommand(rawInput);

		// Check for commands
		if (command == CMD_QUIT) {
			break;
		}
		else if (command == CMD_DEC) {
			if (scientificNotation) {
				scientificNotation = false;
				printf("  Changed to decimal output\n\n> ");
			}
			else {
				printf("  Already in decimal output\n\n> ");
			}
			continue;
		}
		else if (command == CMD_SCI) {
			if (!scientificNotation) {
				scientificNotation = true;
				printf("  Changed to scientific notation output\n\n> ");
			}
			else {
				printf("  Already in scientific notation output\n\n> ");
			}
			continue;
		}
		else if (command == CMD_LS) {
			printf("\n  ans =        %.15E\n", userVarVals[0]);

			printf("\n    CONSTANTS:\n");
			for (int i = 1; i < SESSION_VAR_START && userVars[i][0] != '\0'; i++) {
				printf("  %s   ", userVars[i]);
				
				for (int j = 0; j < 10; j++) {
					if (userVars[i][j] == '\0') {
						printf(" ");
					}
				}

				printf("%.15E\n", userVarVals[i]);
			}

			printf("\n    VARIABLES:\n");
			for (int i = SESSION_VAR_START; i < VARIABLE_SIZE && userVars[i][0] != '\0'; i++) {
				printf("  %s   ", userVars[i]);

				for (int j = 0; j < 10; j++) {
					if (userVars[i][j] == '\0') {
						printf(" ");
					}
				}

				printf("%.15E\n", userVarVals[i]);
			}

			printf("\n> ");
			continue;
		}

		// Perform calculation
		if (error == NO_ERROR) {
			inputToRPN(rawInput, output, variableValues, &error, unrecognizedToken, userVars, userVarVals);
		}
		if (error == NO_ERROR) {
			printVal = evaluateRPN(output, variableValues, &error);
		}

		// Print output, depending on errors and other conditions
		switch (error) {
		case NO_ERROR:
			if (scientificNotation) {
				printf("  %.15E\n", printVal);
			}
			else {
				numDecimals = findNumDecimals(printVal);
				printf("  %.*lf\n", numDecimals, printVal);
			}

			userVarVals[0] = printVal;

			break;
		case ERR_SYNTAX:
			printf("  Syntax error\n");
			break;
		case ERR_UNKNOWN_TOKEN:
			printf("  Unrecognized token \"");
			for (int i = 0; i < UNRECOGNIZED_TOKEN_LAST_ELEMENT; i++) {
				if (unrecognizedToken[i] == '\0') break;
				if (unrecognizedToken[i] == '\t') {
					printf("[tab]");
				}
				else {
					printf("%c", unrecognizedToken[i]);
				}
			}
			if (unrecognizedToken[UNRECOGNIZED_TOKEN_LAST_ELEMENT] != 0) printf("...");

			// check if one of the tokens is a command
			command = findCommand(unrecognizedToken);
			if (command != CMD_NULL) {
				printf("\".  Did you mean to enter a command?\n");
			}
			else printf("\"\n");

			break;
		case ERR_OVERFLOW:
			printf("  Overflow error\n");
			break;
		case ERR_UNDEFINED:
			printf("  Undefined or out of bounds\n");
			break;
		}

		printf("\n> ");
		resetValues(variableValues, output, rawInput, unrecognizedToken, &printVal, &error, &numDecimals);
	}

	return 0;
}

void loadVariables(char userVars[][10], double userVarVals[], int VAR_START_POSITION, int VAR_END_POSITION, char filename[64]) {
	// Loads variables from file into memory
	FILE* file = NULL;
	file = fopen(filename, "r");
	char textInput[LOAD_VAR_HOLDER_SIZE] = { '\0' };
	char holder[LOAD_VAR_HOLDER_SIZE] = { '\0' };
	bool lineError[LOAD_VAR_HOLDER_SIZE] = { false };
	int j = 0; // iterator
	int jHolder = 0;
	bool noLoadFail = true;

	if (file == NULL) {
		printf("  Could not load %s\n", filename);
	}
	else {
		// Iterates line by line. Starts at 1 because element 0 is "ans"
		for (int i = VAR_START_POSITION; i <= VAR_END_POSITION && fgets(textInput, LOAD_VAR_HOLDER_SIZE, file); i++) {
			if ((textInput[0] >= 'a' && textInput[0] <= 'z') || (textInput[0] >= 'A' && textInput[0] <= 'Z')) {
				// Copies variable name
				for (j = 0; j <= 20 && ((textInput[j] >= 'a' && textInput[j] <= 'z') || (textInput[j] >= 'A' && textInput[j] <= 'Z') 
					|| (textInput[j] >= '0' && textInput[j] <= '9') || textInput[j] == '_'); j++) {
					userVars[i][j] = textInput[j];
				}

				// Skips over space.  If character isn't a space, skip this line
				if (textInput[j] != ' ' && textInput[j] != '\t') {
					lineError[i] = true;
					continue;
				}
				else j++;

				// Gets variable values
				for (; j < 118 && ((textInput[j] >= '0' && textInput[j] <= '9') || textInput[j] == '.' || textInput[j] == 'E' 
					|| textInput[j] == '-'); j++) {
					holder[jHolder] = textInput[j];
					jHolder++;
				}
				userVarVals[i] = atof(holder);

				// Clears holder
				while (jHolder > 0) {
					holder[jHolder] = '\0';
					jHolder--;
				}
				holder[jHolder] = '\0';
			}
			else if (textInput[0] == '\0') continue;
			else lineError[i] = true;
		}
	}

	for (int i = 0; i < LOAD_VAR_HOLDER_SIZE; i++) {
		if (lineError[i]) {
			printf("  Failed to parse line %d from file %s\n", i, filename);
			noLoadFail = false;
		}
	}
	if (!noLoadFail) printf("\n");

	if (file != NULL) fclose(file);
}

void saveVariable(char userVars[][10], double userVarVals[], int VAR_START_POSITION, int VAR_END_POSITION, char filename[]) {
	// Saves variables from session to file
	FILE* file = NULL;
	file = fopen(filename, "w");

	if (file == NULL) {
		printf("  Could not load %s\n", filename);
	}
	else {
		for (int i = VAR_START_POSITION; i < VAR_END_POSITION && userVars[i][0] != '\0'; i++) {
			printf("%s", userVars[i]);
			printf(" ");
			printf("%lf\n", userVarVals[i]);
		}
	}

	if (file != NULL) fclose(file);
}

void resetValues(double variableValues[], unsigned char output[], char rawInput[], char unrecognizedToken[],
	double* printVal, int* error, int* numDecimals) {
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
	for (int i = 0; i < INPUT_HOLDER_SIZE; i++) {
		unrecognizedToken[i] = 0;
	}
	*printVal = 0.0;
	*error = 0;
	*numDecimals = 0;
}

int findCommand(char input[]) {
	
	char commands[4][6] = {"quit", "sci", "dec", "ls"};
	int commandCodes[4] = {CMD_QUIT, CMD_SCI, CMD_DEC, CMD_LS};

	for (int i = 0; i < 4; i++) { // command loop
		for (int j = 0; j < 6; j++) { // character loop
			if (commands[i][j] == '\0' && (input[j] == '\0' || input[j] == '\n')) {
				return commandCodes[i];
			}
			if (input[j] != commands[i][j]) {
				// If at any point the input and current command don't match, try next command
				break;
			}
		}

	}
	return CMD_NULL;
}

int findNumDecimals(double input) {
	// Finds appropriate amount of decimals to display such that final output is accurate, and takes up similar amount of space each time
	int decimals = 15;
	while (input >= 10 && decimals > 0) {
		input /= 10.0;
		decimals--;
	}
	return decimals;
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
	return (token > END_OPS && token < END_FUNCS);
}

bool isOperator(unsigned char token) {
	// Returns true if given token is an operator
	return (token >= 0 && token < LEFT_PARENTH);
}

bool isBinaryOperator(unsigned char token) {
	// Returns true if function or operator has two inputs, false if one
	return (token < UNARY_OPERATORS && token != OP_NOT && token != OP_NEG);
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

unsigned char findFunction(char input[]) {
	// Returns the op-code for a given string representing a function. Maximum length is 5. Unused (terminal) characters must be null

	char functions[NR_FUNCTIONS][10] = { "div", "mod", "log", "root",
		"sin", "cos", "tan", "sec", "csc", "cot",
		"asin", "acos", "atan", "asec", "acsc", "acot",
		"sinh", "cosh", "tanh", "sech", "csch", "coth",
		"asinh", "acosh", "atanh", "asech", "acsch", "acoth",
		"sqrt", "ln", "log10", "ceil", "floor", "round", "sgn", "gcd", "lcm", "atan2", "abs", "log2",
		"cbrt", "trunc", "erf", "erfc", "gamma", "hypot", "lgamma", "sinc", "nsinc", "reqll", "perr", "deg", "rad",
		"is", "and", "or", "not", "mod", "xor"
		};
	int functionOP[NR_FUNCTIONS] = { OP_DIV_INT, OP_MOD, OP_LOG, OP_ROOT,
		OP_SIN, OP_COS, OP_TAN, OP_SEC, OP_CSC, OP_COT,
		OP_ASIN, OP_ACOS, OP_ATAN, OP_ASEC, OP_ACSC, OP_ACOT,
		OP_SINH, OP_COSH, OP_TANH, OP_SECH, OP_CSCH, OP_COTH,
		OP_ASINH, OP_ACOSH, OP_ATANH, OP_ASECH, OP_ACSCH, OP_ACOTH,
		OP_SQRT, OP_LN, OP_LOG10, OP_CEIL, OP_FLOOR, OP_ROUND, OP_SIGN, OP_GCD, OP_LCM, OP_ATAN2, OP_ABS, OP_LOG2,
		OP_CBRT, OP_TRUNC, OP_ERF, OP_ERFC, OP_GAMMA, OP_HYPOT, OP_LGAMMA, OP_SINC, OP_NSINC, OP_REQLL, OP_PERR, OP_DEG, OP_RAD,
		OP_IS, OP_AND, OP_OR, OP_NOT, OP_MOD, OP_XOR
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

double findVariable(char input[], int* error, char unrecognizedToken[], char userVars[][10], double userVarVals[]) {

	for (int i = 0; i < VARIABLE_SIZE; i++) { // variable loop
		for (int j = 0; j < 10; j++) { // character loop
			if (input[j] != userVars[i][j]) {
				// If at any point the input and current variable don't match, try next variable
				break;
			}
			else if (j == 9 && input[10] == '\0') {
				// Return if end of the input has been reached and all have matched
				return userVarVals[i];
			}
		}
	}
	*error = ERR_UNKNOWN_TOKEN;
	for (int i = 0; i < INPUT_HOLDER_SIZE; i++) {
		unrecognizedToken[i] = input[i];
	}

	return 0.0;
}

unsigned char tokenize(char rawInput[], int* indexPtr, double variableValues[], int* varValIndex, bool unaryNegation,
	int* error, char unrecognizedToken[], char userVars[][10], double userVarVals[]) {
	// Converts multi-character inputs (such as function names) into their representative tokens

	char currChar = rawInput[*indexPtr];
	char inputHolder[INPUT_HOLDER_SIZE] = { 0 };
	int inputHolderIndex = 0;
	int numDecimalPlaces = 0; // If this exeeds one for a number, error
	int numE = 0;
	bool prevCharIsE = false;
	bool negativeAnswerIndex = false;
	unsigned char outputToken = 0;
	int answerNumber = -1;

	if ((currChar >= '0' && currChar <= '9')) {
		// If token is a value, scan all characters until no longer part of a value
		while ((currChar >= '0' && currChar <= '9') || currChar == '.' || currChar == 'E' || currChar == '-') {
			if (currChar == '-') {
				if (!prevCharIsE) {
					break;
				}
			}
			prevCharIsE = false;

			if (currChar == '.') {
				if (numE > 0) {
					*error = ERR_SYNTAX;
					printf("A");
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
				*error = ERR_OVERFLOW;
				break;
			}

			inputHolder[inputHolderIndex] = currChar;
			inputHolderIndex++;
			(*indexPtr)++;
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
			currChar = rawInput[*indexPtr];
		}

		outputToken = findFunction(inputHolder);

		// If token wasn't a function, test for variables
		if (outputToken == OP_NULL) {
			variableValues[*varValIndex] = findVariable(inputHolder, error, unrecognizedToken, userVars, userVarVals);
			outputToken = *varValIndex + VAR_BEGIN;
			(*varValIndex)++;
		}
	}
	else if (currChar == '<') {
		(*indexPtr)++;
		currChar = rawInput[*indexPtr];

		if (currChar == '=') {
			outputToken = OP_LESS_THAN_EQUAL_TO;
			(*indexPtr)++;
		}
		else if (currChar == '-') {
			(*indexPtr)++;
			currChar = rawInput[*indexPtr];

			if (currChar == '>') {
				outputToken == OP_IFF;
				(*indexPtr)++;
			}
			else {
				outputToken = OP_IMPLIED_BY;
			}
		}
		else {
			outputToken = OP_LESS_THAN;
		}
	}
	else if (currChar == '>') {
		(*indexPtr)++;
		currChar = rawInput[*indexPtr];

		if (currChar == '=') {
			outputToken = OP_GREATER_THAN_EQUAL_TO;
			(*indexPtr)++;
		}
		else {
			outputToken = OP_GREATER_THAN;
		}
	}
	else if (currChar == '-') {
		(*indexPtr)++;
		currChar = rawInput[*indexPtr];

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

void pushOperator(unsigned char token, unsigned char stack[], int* stackLength, unsigned char output[], int* outputLength, int* error) {
	// Pushes given token and some tokens on stack to the output in such a manner that output is in postfix

	int topOfStack = 0;

	int precedence[20] = {
		0, // NULL
		7, // ADD
		7, // SUB
		8, // MUL
		8, // DIV
		8, // MOD
		9, // NEG
		5, // IS 
		6, // GREATER_THAN 
		6, // LESS_THAN 
		6, // GREATER_THAN_OR_EQUAL_TO 
		6, // LESS_THAN_OR_EQUAL_TO
		3, // AND
		1, // OR
		9, // NOT 
		2, // XOR
		4, // IMPLIES
		4, // IFF 
		4, // IMPLIED BY
		10,// EXP
	};

	int stackPrecedence = 0; int tokenPrecedence = 0;

	if (stackIsEmpty(stack)) {
		push(stack, token, stackLength, STACK_SIZE, error);
		if (*error != NO_ERROR) return;
	}
	else {
		// Push all higher precedence tokens from stack to output, then push current token to stack
		while (!stackIsEmpty(stack)) {
			topOfStack = stack[*stackLength - 1];
			if (topOfStack < END_FUNCS) {
				if (topOfStack < END_OPS) {
					// Operations
					stackPrecedence = precedence[topOfStack];
					tokenPrecedence = precedence[token];

					if ((token != OP_EXP && tokenPrecedence <= stackPrecedence)
						|| (token == OP_EXP && tokenPrecedence < stackPrecedence)) {
						push(output, pop(stack, stackLength), outputLength, OUTPUT_SIZE, error);
						if (*error != NO_ERROR) return;
					}
					else break;
				}
				else {
					// Functions, being in prefix rather than infix notation, are simply pushed
					push(output, pop(stack, stackLength), outputLength, OUTPUT_SIZE, error);
					if (*error != NO_ERROR) return;
				}
			}
			else {
				// Neither operators nor functions (parentheses and such)
				break;
			}
		}
		push(stack, token, stackLength, STACK_SIZE, error);
	}
}

void inputToRPN(char rawInput[], unsigned char output[], double variableValues[], int* error, char unrecognizedToken[],
	char userVars[][10], double userVarVals[]) {
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
		token = tokenize(rawInput, &index, variableValues, &varValIndex, unaryNegation, error, unrecognizedToken, userVars, userVarVals);
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
		else if (token == OP_NEG || token == OP_NOT) {
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
		
		// TESTING
		/*for (int i = 0; i < 0; i++) {
			printf("%d ", output[i]);
		}*/

		if (*error != NO_ERROR) return;
	}
}

double evaluateRPN(unsigned char input[], double variableValues[], int* error) {
	// Given input in RPN format and a list of variable values, performs the actual calculations of the program 
	// in the given order and returns the final result

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
					if (variableValues[valueIndexRight] == 0.0) {
						*error = ERR_UNDEFINED;
						return 0.0;
					}
					variableValues[valueIndexLeft] = variableValues[valueIndexLeft] / variableValues[valueIndexRight];
					break;
				case OP_EXP:
					variableValues[valueIndexLeft] = pow(variableValues[valueIndexLeft], variableValues[valueIndexRight]);
					break;
				case OP_DIV_INT:
					variableValues[valueIndexLeft] = (double)(doubleToInt(variableValues[valueIndexLeft])
						/ doubleToInt(variableValues[valueIndexRight]));
					break;
				case OP_MOD:
					if (doubleToInt(variableValues[valueIndexRight]) == 0) {
						*error = ERR_UNDEFINED;
						return 0.0;
					}
					else
						variableValues[valueIndexLeft] = (double)(doubleToInt(variableValues[valueIndexLeft])
							% doubleToInt(variableValues[valueIndexRight]));
					break;
				case OP_GCD:
					variableValues[valueIndexLeft] = gcd(variableValues[valueIndexRight], variableValues[valueIndexLeft]);
					break;
				case OP_LCM:
					variableValues[valueIndexLeft] = (variableValues[valueIndexRight] /
						gcd(variableValues[valueIndexRight], variableValues[valueIndexLeft])) *
						variableValues[valueIndexLeft];
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
					if (variableValues[valueIndexRight] >= 0.0)
						variableValues[valueIndexRight] = 1.0;
					else
						variableValues[valueIndexRight] = -1.0;
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
				case OP_LOG10:
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
					else
						variableValues[valueIndexRight] = pi / 2;
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
				case OP_ATAN2:
					variableValues[valueIndexLeft] = atan2(variableValues[valueIndexLeft], variableValues[valueIndexRight]);
					break;
				case OP_SINC:
					if (variableValues[valueIndexRight] == 0.0) variableValues[valueIndexRight] = 1.0;
					else variableValues[valueIndexRight] = sin(variableValues[valueIndexRight]) / variableValues[valueIndexRight];
					break;
				case OP_NSINC:
					if (variableValues[valueIndexRight] == 0.0) variableValues[valueIndexRight] = 1.0;
					else variableValues[valueIndexRight] = sin(pi * variableValues[valueIndexRight])
						/ (pi * variableValues[valueIndexRight]);
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
				case OP_LGAMMA:
					variableValues[valueIndexRight] = lgamma(variableValues[valueIndexRight]);
					break;
				case OP_REQLL:
					variableValues[valueIndexLeft] = (variableValues[valueIndexLeft] * variableValues[valueIndexRight])
						/ (variableValues[valueIndexLeft] + variableValues[valueIndexRight]);
					break;
				case OP_PERR:
					variableValues[valueIndexLeft] = 100 * (fabs(variableValues[valueIndexLeft] -
						variableValues[valueIndexRight]) / variableValues[valueIndexRight]);
					break;
				case OP_DEG:
					variableValues[valueIndexRight] *= RAD_TO_DEG_CONST;
					break;
				case OP_RAD:
					variableValues[valueIndexRight] *= DEG_TO_RAD_CONST;
					break;
				case OP_LOG2:
					variableValues[valueIndexRight] = log2(variableValues[valueIndexRight]);
					break;
				case OP_IS:
					variableValues[valueIndexLeft] = variableValues[valueIndexLeft] == variableValues[valueIndexRight];
					break;
				case OP_GREATER_THAN:
					variableValues[valueIndexLeft] = variableValues[valueIndexLeft] > variableValues[valueIndexRight];
					break;
				case OP_GREATER_THAN_EQUAL_TO:
					variableValues[valueIndexLeft] = variableValues[valueIndexLeft] >= variableValues[valueIndexRight];
					break;
				case OP_LESS_THAN:
					variableValues[valueIndexLeft] = variableValues[valueIndexLeft] < variableValues[valueIndexRight];
					break;
				case OP_LESS_THAN_EQUAL_TO:
					variableValues[valueIndexLeft] = variableValues[valueIndexLeft] <= variableValues[valueIndexRight];
					break;
				case OP_AND:
					variableValues[valueIndexLeft] = variableValues[valueIndexLeft] && variableValues[valueIndexRight];
					break;
				case OP_OR:
					variableValues[valueIndexLeft] = variableValues[valueIndexLeft] || variableValues[valueIndexRight];
					break;
				case OP_NOT:
					variableValues[valueIndexRight] = !(variableValues[valueIndexRight]);
					break;
				case OP_XOR:
					variableValues[valueIndexLeft] = !(variableValues[valueIndexLeft]) != !(variableValues[valueIndexRight]);
					break;
				case OP_IMPLIES:
					variableValues[valueIndexLeft] = !(variableValues[valueIndexLeft]) || variableValues[valueIndexRight];
					break;
				case OP_IFF:
					variableValues[valueIndexLeft] = !(variableValues[valueIndexLeft]) == !(variableValues[valueIndexRight]);
					break;
				case OP_IMPLIED_BY:
					variableValues[valueIndexLeft] = variableValues[valueIndexLeft] && !variableValues[valueIndexRight];
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
	if (isnan(variableValues[0]) || isinf(variableValues[0])) {
		*error = ERR_UNDEFINED;
	}
	return variableValues[0];
}