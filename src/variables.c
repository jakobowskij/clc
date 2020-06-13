#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include "constants.h"
#include "variables.h"
#include "global.h"

void loadVariables(int VAR_START_POSITION, int VAR_END_POSITION, char filename[64]) {
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

				// Gets variable name
				for (j = 0; j <= 20 && ((textInput[j] >= 'a' && textInput[j] <= 'z') || (textInput[j] >= 'A' && textInput[j] <= 'Z')
					|| (textInput[j] >= '0' && textInput[j] <= '9') || textInput[j] == '_'); j++) {
					variableNames[i][j] = textInput[j];
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

				// Puts variable values into memory
				variableMap[i] = atof(holder);
				// TODO:  Add support for different variable types

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

// Saves variables to file
void saveVariable(int VAR_START_POSITION, int VAR_END_POSITION, char filename[]) {
	// Start and end positions will be different depending on whether constants or regular variables are being saved
	// Positions should be chosen such that unnamed variables are not saved

	FILE* file = NULL;
	file = fopen(filename, "w");

	if (file == NULL) {
		printf("  Could not load %s\n", filename);
	}
	else {
		for (int i = VAR_START_POSITION; i < VAR_END_POSITION && variableNames[i][0] != '\0'; i++) {
			printf("%s", variableNames[i]);
			printf(" ");
			printf("%lf\n", variableMap[i]);
		}
	}

	if (file != NULL) fclose(file);
}

// Allocates memory for variable, assigns variable
int addVariable() {


}

// Memory for particular variable is freed.  TODO: Auto-defrag?
void delVariable() {


}

// Finds variable's value from its name, and returns it
// TODO: Will need heavy modification when multiple data types will need to be supported
// TODO: Will need heavy modification when new instruction compilation scheme is implemented
double findVariable(char input[]) {

	for (int i = 0; i < VAR_NAME_SIZE; i++) { // variable loop
		for (int j = 0; j < 10; j++) { // character loop
			if (input[j] != variableNames[i][j]) {
				// If at any point the input and current variable don't match, try next variable
				break;
			}
			else if (j == 9 && input[10] == '\0') {
				// Return if end of the input has been reached and all have matched
				return variableMap[i];
			}
		}
	}
	error = ERR_UNKNOWN_TOKEN;
	for (int i = 0; i < INPUT_HOLDER_SIZE; i++) {
		unrecognizedToken[i] = input[i];
	}

	return 0.0;
}