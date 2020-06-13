#ifndef AUXILIARY_H
#define AUXILIARY_H

int findNumDecimals(double input);
unsigned int pop(unsigned int arr[], int* length);
void push(unsigned int arr[], unsigned int val, int* length, int maxLength);
bool stackIsEmpty(unsigned int stack[]);
bool isFunction(unsigned int token);
bool isOperator(unsigned int token);
bool isBinaryOperator(unsigned int token);
long long int doubleToInt(double input);
double gcd(double a, double b);
unsigned int findFunction(char input[]);
void resetValues(double* printVal);

#endif