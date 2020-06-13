#ifndef VARIABLES_H
#define VARIABLES_H

void loadVariables(int VAR_START_POSITION, int VAR_END_POSITION, char filename[64]);
void saveVariable(int VAR_START_POSITION, int VAR_END_POSITION, char filename[]);
int addVariable();
void delVariable();
double findVariable(char input[]);

#endif
