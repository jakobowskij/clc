#ifndef RPN_H
#define RPN_H

void pushOperator(unsigned int token, unsigned int stack[], int* stackLength, int* outputLength);
void inputToRPN();
double evaluateRPN();

#endif
