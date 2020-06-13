#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include "constants.h"
#include "auxiliary.h"
#include "tokenize.h"
#include "rpn.h"
#include "global.h"

// Pushes given token and some tokens on stack to the output such that output is in postfix
void pushOperator(unsigned int token, unsigned int stack[], int* stackLength, int* outputLength) {

	int topOfStack = 0;

	int precedence[26] = {
		0,  // NULL	
		10, // ADD
		10, // SUB
		11, // MUL
		11, // DIV
		11, // MOD
		12, // NEG
		8,  // IS
		8,  // GREATER_THAN 
		8,  // LESS_THAN 
		8,  // GREATER_THAN_OR_EQUAL_TO 
		8,  // LESS_THAN_OR_EQUAL_TO
		4,  // AND
		2,  // OR
		12, // NOT 
		3,  // XOR
		1,  // IMPLIES
		1,  // IFF 
		1,  // IMPLIED BY
		9,  // LEFT SHIFT
		9,  // RIGHT SHIFT
		4,  // BITWISE AND
		5,  // BITWISE OR
		12, // BITWISE NOT
		6,  // BITWISE XOR
		14, // EXP
	};

	int stackPrecedence = 0; int tokenPrecedence = 0;

	if (stackIsEmpty(stack)) {
		push(stack, token, stackLength, STACK_SIZE);
		if (error != NO_ERROR) return;
	}
	else {
		// Push all higher precedence tokens from stack to output, then push current token to stack
		while (!stackIsEmpty(stack)) {
			topOfStack = stack[*stackLength - 1];
			if (topOfStack < END_FUNCS) {
				if (topOfStack < END_OPS) {
					// Operations.  Are offset because operators start at OP_NULL
					stackPrecedence = precedence[topOfStack - OP_NULL];
					tokenPrecedence = precedence[token - OP_NULL];

					if ((token != OP_EXP && tokenPrecedence <= stackPrecedence)
						|| (token == OP_EXP && tokenPrecedence < stackPrecedence)) {
						push(expressionRPN, pop(stack, stackLength), outputLength, RPN_SIZE);
						if (error != NO_ERROR) return;
					}
					else break;
				}
				else {
					// Functions, being in prefix rather than infix notation, are simply pushed
					push(expressionRPN, pop(stack, stackLength), outputLength, RPN_SIZE);
					if (error != NO_ERROR) return;
				}
			}
			else {
				// Neither operators nor functions (parentheses, etc)
				break;
			}
		}
		push(stack, token, stackLength, STACK_SIZE);
	}
}

void inputToRPN() {
	// Converts infix input to array of postfix tokens

	unsigned int stack[STACK_SIZE] = { 0 }; int stackLength = 0;
	int outputLength = 0;
	unsigned int token = 0;
	int keywordState = KWS_READY;
	int indentCnt = 0;
	int evalVarHead = EVAL_VARS_START;

	bool implicitMultiplication = false;
	// Whether or not the next token has the ability to be implicitly multiplied, such as with parentheses: 3(5) = 15

	bool unaryNegation = true;
	// Whether the next '-' encountered should be interpreted as negation (true) or subtraction (false)

	int index = 0;

	// Count indentation
	while (index < INPUT_SIZE && terminalInput[index] == '\t') {
		indentCnt++;
		index++;
	}

	while (index < INPUT_SIZE) {
		if (terminalInput[index] == '\n') {
			// If end of input has been reached
			break;
		}
		token = tokenize(&index, &evalVarHead, unaryNegation, &keywordState);
		if (error != NO_ERROR) return;
		if (token == OP_NULL) continue;

		else if (token == ARG_SEPARATOR) {
			// Comma that separates function arguments.  Operators are popped until left parentheses encountered
			while (stack[stackLength - 1] != LEFT_PARENTH && !stackIsEmpty(stack)) {
				push(expressionRPN, pop(stack, &stackLength), &outputLength, RPN_SIZE);
				if (error != 0) return;
			}
			unaryNegation = true;
			implicitMultiplication = false;
		}
		else if (token == OP_NEG || token == OP_NOT) {
			// Negation may be applied to number to its right directly after an operator, so cannot pop any operators from stack
			push(stack, token, &stackLength, STACK_SIZE);
		}
		else if (isFunction(token)) {
			if (implicitMultiplication) {
				pushOperator(OP_MUL, stack, &stackLength, &outputLength);
			}
			push(stack, token, &stackLength, STACK_SIZE);
			implicitMultiplication = false;
			unaryNegation = true;
		}
		else if (isOperator(token)) {
			// Non-function operators
			pushOperator(token, stack, &stackLength, &outputLength);
			implicitMultiplication = false;
			unaryNegation = true;
		}
		else if (token == LEFT_PARENTH) {
			if (implicitMultiplication) {
				pushOperator(OP_MUL, stack, &stackLength, &outputLength);
			}
			push(stack, LEFT_PARENTH, &stackLength, STACK_SIZE);
			implicitMultiplication = false;
			unaryNegation = true;
		}
		else if (token == RIGHT_PARENTH) {
			// Pop operators until left parentheses encountered, then pop the left parenthesis
			if (!stackIsEmpty(stack)) {
				while (stack[stackLength - 1] != LEFT_PARENTH && !stackIsEmpty(stack)) {
					push(expressionRPN, pop(stack, &stackLength), &outputLength, RPN_SIZE);
					if (error != NO_ERROR) return;
				}
				pop(stack, &stackLength);
			}
		}
		else {
			// If token is a variable
			if (implicitMultiplication) {
				pushOperator(OP_MUL, stack, &stackLength, &outputLength);
			}
			push(expressionRPN, token, &outputLength, RPN_SIZE);
			implicitMultiplication = true;
			unaryNegation = false;
		}
	}

	while (!(stack[0] == 0)) {
		// Push all remaining operators from stack
		push(expressionRPN, pop(stack, &stackLength), &outputLength, RPN_SIZE);
		if (error != NO_ERROR) return;
	}
}

// Given input in RPN format and a list of variable values, performs the actual calculations of the program 
// in the given order and returns the final result
double evaluateRPN() {

	// ============= TEST =============

	unsigned int stack[STACK_SIZE] = { 0 }; int stackLength = 0;
	int valueIndexLeft = 0; int valueIndexRight = 0;
	unsigned int operand = 0;
	bool binary;

	for (int indexRPN = 0; indexRPN < RPN_SIZE; indexRPN++) {
		if (expressionRPN[indexRPN] == 0) {
			break;
		}
		binary = false;

		// If current input is variable or there is no operator on the stack, the input is pushed
		if (expressionRPN[indexRPN] < OPERATOR_START || stackIsEmpty(stack) || (stack[stackLength - 1] > OPERATOR_START && stack[stackLength - 1] < END_FUNCS)) {
			push(stack, expressionRPN[indexRPN], &stackLength, STACK_SIZE);
			if (error != NO_ERROR) return 0.0;
		}
		// Otherwise, the operation atop the stack is executed using the variables directly below it; lower variable recieves the result
		else {
			push(stack, expressionRPN[indexRPN], &stackLength, STACK_SIZE);
			if (error != NO_ERROR) return 0.0;
			if (stackLength >= 2) {
				operand = stack[stackLength - 1];
				valueIndexRight = stack[stackLength - 2];
				if (isBinaryOperator(operand)) {
					// Binary operators will act on two values
					// TODO: Change to a loop to support functions of any number of variables
					if (stackLength >= 3) {
						valueIndexLeft = stack[stackLength - 3];
						binary = true;
					}
					else {
						error = ERR_SYNTAX;
						return 0.0;
					}
				}
				switch (operand) {
				case OP_ADD:
					variableMap[valueIndexLeft] = variableMap[valueIndexLeft] + variableMap[valueIndexRight];
					break;
				case OP_SUB:
					variableMap[valueIndexLeft] = variableMap[valueIndexLeft] - variableMap[valueIndexRight];
					break;
				case OP_NEG:
					variableMap[valueIndexRight] = -(variableMap[valueIndexRight]);
					break;
				case OP_MUL:
					variableMap[valueIndexLeft] = variableMap[valueIndexLeft] * variableMap[valueIndexRight];
					break;
				case OP_DIV:
					if (variableMap[valueIndexRight] == 0.0) {
						error = ERR_UNDEFINED;
						return 0.0;
					}
					variableMap[valueIndexLeft] = variableMap[valueIndexLeft] / variableMap[valueIndexRight];
					break;
				case OP_EXP:
					variableMap[valueIndexLeft] = pow(variableMap[valueIndexLeft], variableMap[valueIndexRight]);
					break;
				case OP_DIV_INT:
					variableMap[valueIndexLeft] = (double)(doubleToInt(variableMap[valueIndexLeft])
						/ doubleToInt(variableMap[valueIndexRight]));
					break;
				case OP_MOD:
					if (doubleToInt(variableMap[valueIndexRight]) == 0) {
						error = ERR_UNDEFINED;
						return 0.0;
					}
					else
						variableMap[valueIndexLeft] = (double)(doubleToInt(variableMap[valueIndexLeft])
							% doubleToInt(variableMap[valueIndexRight]));
					break;
				case OP_GCD:
					variableMap[valueIndexLeft] = gcd(variableMap[valueIndexRight], variableMap[valueIndexLeft]);
					break;
				case OP_LCM:
					variableMap[valueIndexLeft] = (variableMap[valueIndexRight] /
						gcd(variableMap[valueIndexRight], variableMap[valueIndexLeft])) *
						variableMap[valueIndexLeft];
					break;
				case OP_CEIL:
					variableMap[valueIndexRight] = ceil(variableMap[valueIndexRight]);
					break;
				case OP_FLOOR:
					variableMap[valueIndexRight] = floor(variableMap[valueIndexRight]);
					break;
				case OP_ROUND:
					variableMap[valueIndexRight] = round(variableMap[valueIndexRight]);
					break;
				case OP_TRUNC:
					variableMap[valueIndexRight] = trunc(variableMap[valueIndexRight]);
					break;
				case OP_SIGN:
					if (variableMap[valueIndexRight] >= 0.0)
						variableMap[valueIndexRight] = 1.0;
					else
						variableMap[valueIndexRight] = -1.0;
					break;
				case OP_ABS:
					variableMap[valueIndexRight] = fabs(variableMap[valueIndexRight]);
					break;
				case OP_LOG:
					variableMap[valueIndexLeft] = log10(variableMap[valueIndexRight]) / log10(variableMap[valueIndexLeft]);
					break;
				case OP_LN:
					variableMap[valueIndexRight] = log(variableMap[valueIndexRight]);
					break;
				case OP_LOG10:
					variableMap[valueIndexRight] = log10(variableMap[valueIndexRight]);
					break;
				case OP_ROOT:
					variableMap[valueIndexLeft] = pow(variableMap[valueIndexRight], (1 / variableMap[valueIndexLeft]));
					break;
				case OP_SQRT:
					variableMap[valueIndexRight] = sqrt(variableMap[valueIndexRight]);
					break;
				case OP_CBRT:
					variableMap[valueIndexRight] = cbrt(variableMap[valueIndexRight]);
					break;
				case OP_HYPOT:
					variableMap[valueIndexLeft] = hypot(variableMap[valueIndexLeft], variableMap[valueIndexRight]);
					break;
				case OP_SIN:
					variableMap[valueIndexRight] = sin(variableMap[valueIndexRight]);
					break;
				case OP_COS:
					variableMap[valueIndexRight] = cos(variableMap[valueIndexRight]);
					break;
				case OP_TAN:
					variableMap[valueIndexRight] = tan(variableMap[valueIndexRight]);
					break;
				case OP_SEC:
					variableMap[valueIndexRight] = 1 / cos(variableMap[valueIndexRight]);
					break;
				case OP_CSC:
					variableMap[valueIndexRight] = 1 / sin(variableMap[valueIndexRight]);
					break;
				case OP_COT:
					variableMap[valueIndexRight] = 1 / tan(variableMap[valueIndexRight]);
					break;
				case OP_ASIN:
					variableMap[valueIndexRight] = asin(variableMap[valueIndexRight]);
					break;
				case OP_ACOS:
					variableMap[valueIndexRight] = acos(variableMap[valueIndexRight]);
					break;
				case OP_ATAN:
					variableMap[valueIndexRight] = atan(variableMap[valueIndexRight]);
					break;
				case OP_ASEC:
					variableMap[valueIndexRight] = acos(1 / (variableMap[valueIndexRight]));
					break;
				case OP_ACSC:
					variableMap[valueIndexRight] = asin(1 / (variableMap[valueIndexRight]));
					break;
				case OP_ACOT:
					if (variableMap[valueIndexRight] > 0) {
						variableMap[valueIndexRight] = atan(1 / variableMap[valueIndexRight]);
					}
					else if (variableMap[valueIndexRight] < 0) {
						variableMap[valueIndexRight] = atan(1 / variableMap[valueIndexRight]) + pi;
					}
					else variableMap[valueIndexRight] = pi / 2;
					break;
				case OP_SINH:
					variableMap[valueIndexRight] = sinh(variableMap[valueIndexRight]);
					break;
				case OP_COSH:
					variableMap[valueIndexRight] = cosh(variableMap[valueIndexRight]);
					break;
				case OP_TANH:
					variableMap[valueIndexRight] = tanh(variableMap[valueIndexRight]);
					break;
				case OP_SECH:
					variableMap[valueIndexRight] = 1 / cosh(variableMap[valueIndexRight]);
					break;
				case OP_CSCH:
					variableMap[valueIndexRight] = 1 / sinh(variableMap[valueIndexRight]);
					break;
				case OP_COTH:
					variableMap[valueIndexRight] = 1 / tanh(variableMap[valueIndexRight]);
					break;
				case OP_ASINH:
					variableMap[valueIndexRight] = asinh(variableMap[valueIndexRight]);
					break;
				case OP_ACOSH:
					variableMap[valueIndexRight] = acosh(variableMap[valueIndexRight]);
					break;
				case OP_ATANH:
					variableMap[valueIndexRight] = atanh(variableMap[valueIndexRight]);
					break;
				case OP_ASECH:
					variableMap[valueIndexRight] = acosh(1 / (variableMap[valueIndexRight]));
					break;
				case OP_ACSCH:
					variableMap[valueIndexRight] = asinh(1 / (variableMap[valueIndexRight]));
					break;
				case OP_ACOTH:
					variableMap[valueIndexRight] = atanh(1 / (variableMap[valueIndexRight]));
					break;
				case OP_ATAN2:
					variableMap[valueIndexLeft] = atan2(variableMap[valueIndexLeft], variableMap[valueIndexRight]);
					break;
				case OP_SINC:
					if (variableMap[valueIndexRight] == 0.0) variableMap[valueIndexRight] = 1.0;
					else variableMap[valueIndexRight] = sin(variableMap[valueIndexRight]) / variableMap[valueIndexRight];
					break;
				case OP_NSINC:
					if (variableMap[valueIndexRight] == 0.0) variableMap[valueIndexRight] = 1.0;
					else variableMap[valueIndexRight] = sin(pi * variableMap[valueIndexRight])
						/ (pi * variableMap[valueIndexRight]);
					break;
				case OP_ERF:
					variableMap[valueIndexRight] = erf(variableMap[valueIndexRight]);
					break;
				case OP_ERFC:
					variableMap[valueIndexRight] = erfc(variableMap[valueIndexRight]);
					break;
				case OP_GAMMA:
					variableMap[valueIndexRight] = tgamma(variableMap[valueIndexRight]);
					break;
				case OP_LGAMMA:
					variableMap[valueIndexRight] = lgamma(variableMap[valueIndexRight]);
					break;
				case OP_REQLL:
					variableMap[valueIndexLeft] = (variableMap[valueIndexLeft] * variableMap[valueIndexRight])
						/ (variableMap[valueIndexLeft] + variableMap[valueIndexRight]);
					break;
				case OP_PERR:
					variableMap[valueIndexLeft] = 100 * (fabs(variableMap[valueIndexLeft] -
						variableMap[valueIndexRight]) / variableMap[valueIndexRight]);
					break;
				case OP_DEG:
					variableMap[valueIndexRight] *= RAD_TO_DEG_CONST;
					break;
				case OP_RAD:
					variableMap[valueIndexRight] *= DEG_TO_RAD_CONST;
					break;
				case OP_LOG2:
					variableMap[valueIndexRight] = log2(variableMap[valueIndexRight]);
					break;
				case OP_IS:
					variableMap[valueIndexLeft] = variableMap[valueIndexLeft] == variableMap[valueIndexRight];
					break;
				case OP_GREATER_THAN:
					variableMap[valueIndexLeft] = variableMap[valueIndexLeft] > variableMap[valueIndexRight];
					break;
				case OP_GREATER_THAN_EQUAL_TO:
					variableMap[valueIndexLeft] = variableMap[valueIndexLeft] >= variableMap[valueIndexRight];
					break;
				case OP_LESS_THAN:
					variableMap[valueIndexLeft] = variableMap[valueIndexLeft] < variableMap[valueIndexRight];
					break;
				case OP_LESS_THAN_EQUAL_TO:
					variableMap[valueIndexLeft] = variableMap[valueIndexLeft] <= variableMap[valueIndexRight];
					break;
				case OP_AND:
					variableMap[valueIndexLeft] = variableMap[valueIndexLeft] && variableMap[valueIndexRight];
					break;
				case OP_OR:
					variableMap[valueIndexLeft] = variableMap[valueIndexLeft] || variableMap[valueIndexRight];
					break;
				case OP_NOT:
					variableMap[valueIndexRight] = !(variableMap[valueIndexRight]);
					break;
				case OP_XOR:
					variableMap[valueIndexLeft] = !(variableMap[valueIndexLeft]) != !(variableMap[valueIndexRight]);
					break;
				case OP_IMPLIES:
					variableMap[valueIndexLeft] = !(variableMap[valueIndexLeft]) || variableMap[valueIndexRight];
					break;
				case OP_IFF:
					variableMap[valueIndexLeft] = !(variableMap[valueIndexLeft]) == !(variableMap[valueIndexRight]);
					break;
				case OP_IMPLIED_BY:
					variableMap[valueIndexLeft] = variableMap[valueIndexLeft] && !variableMap[valueIndexRight];
					break;
				default:
					error = ERR_SYNTAX;
					return 0.0;
				}

				pop(stack, &stackLength);
				if (binary) {
					pop(stack, &stackLength);
				}
			}
			else {
				error = ERR_SYNTAX;
				return 0.0;
			}
		}
	}

	if ((stack[0] > OP_NULL && stack[0] < END_FUNCS) || stack[1] != 0) {
		// If a function is left on the stack, there were too many operators
		// If there is more than one thing left on the stack, an error occurred (most likely a misplaced argument separator)
		error = ERR_SYNTAX;
	}
	if (isnan(variableMap[0]) || isinf(variableMap[0])) {
		error = ERR_UNDEFINED;
	}
	return variableMap[EVAL_VARS_START];
}