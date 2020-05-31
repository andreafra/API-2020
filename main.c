#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_INPUT 1024 + 1
#define CMD_QUIT "q"

// Prototypes
void parseInput();
void parseSingleCmd();
void parseDoubleCmd();

// Data structures
typedef enum command_type_ {
	CHANGE,
	DELETE,
	PRINT
} CmdType;

// A list element representing a row of text
// To be used in a single-linked list
typedef struct line_ {
	char text[MAX_INPUT];
	Line* next;
} Line;

// A list element representing a command
// To be used in a double-linked list
typedef struct command_ {
	CmdType type;
	int addr1;
	int addr2;
	Cmd* prev;
	Cmd* next;
} Cmd;

// Global Variables
// User input
char inputStr[MAX_INPUT];

// 1 = parse Commands, 0 = handle text
int isCmd = 1;

// Parameters of a command
int addr1, addr2, times;

// the file buffer
Line* buffer = NULL;

// the queues to perform undo/redo
Cmd* undoQueue = NULL;
Cmd* redoQueue = NULL;

// Entrypoint
int main() {
	do {
		fgets(inputStr, MAX_INPUT, stdin);
		parseInput();
	} while(1);
	
	return 0;
}

void parseInput() {
	// I need to parse the command
	if(isCmd) {
		// Get the lenght of the input
		size_t len = strlen(inputStr) - 1;
		// Get command char (the identifier)
		char cmd = inputStr[len - 1];
		// "Delete" the cmd char
		inputStr[len - 1] = '\0';
		// Call the right parser for each command
		switch (cmd) {
			case 'c': // Change
				parseDoubleCmd();
				isCmd = 0;
				break;
			case 'd': // Delete
				parseDoubleCmd();
				isCmd = 1;
				break;
			case 'p': // Print
				parseDoubleCmd();
				isCmd = 1;
				break;
			case 'u': // Undo
				parseSingleCmd();
				break;
			case 'r': // Redo
				parseSingleCmd();
				break;
			case 'q':
				exit(0);
				return;
			default:
				break;
		}
	} else {
		// I just need to handle the content
	}
}

// If you're getting weird values and need fixes, check 
// https://stackoverflow.com/questions/5493235
void parseSingleCmd() {
	times = strtol(inputStr, NULL, 10);
	printf("> %d\n", times);
}

void parseDoubleCmd() {
	char* token1 = strtok(inputStr, ",");
	char* token2 = strtok(NULL, ",");
	addr1 = strtol(token1, NULL, 10);
	addr2 = strtol(token2, NULL, 10);
	printf("> %d, %d\n", addr1, addr2);
}
