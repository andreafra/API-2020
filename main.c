#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

// #define NDEBUG
#include <assert.h>

#define MAX_INPUT_SIZE 1024


typedef enum _CmdType { CHANGE, DELETE } CmdType;

typedef struct _Line {
	char *text;
	struct _Line *next;
} Line;

typedef struct _Cmd {
	CmdType type;
	int addr1;
	int addr2;
	Line *old;
	Line *new;
	struct _Cmd *next;
} Cmd;

// The start of the file to write.
Line *BUFFER = NULL;

// Add a command to this stack when you DO/REDO a command.
Cmd *UNDO_STACK = NULL;
int UNDO_STACK_SIZE = 0;
// Add a command to this stack when you UNDO a command.
Cmd *REDO_STACK = NULL;
int REDO_STACK_SIZE = 0;

// Set it to true to terminate the program.
bool SHOULD_QUIT = false;

char INPUT_STRING[MAX_INPUT_SIZE];

// PROTOTYPES

// Read from stdin and handle commands.
void parseInput();

// Commands from input

void change(int addr1, int addr2);
void delete(int addr1, int addr2);
void print(int addr1, int addr2);
void undo(int times);
void redo(int times);

// External buffer data structure commands
void updateLines();
void deleteLines();
void insertLines();
void printLines();

int main() {
	while(!SHOULD_QUIT) {
		// Each iteration is a command
		parseInput();		
	}

	return 0;
}

// Save a string with the minimum size possible.
char *newText(char *text) {
	// \n should be excluded from the string
	assert(text[strlen(text)-1] != '\n');
	char *allocatedStr = malloc(strlen(text) * sizeof(char) + 1);
	assert(allocatedStr != NULL);
	strcpy(allocatedStr, text);
	return allocatedStr;
}

// Create a new Line by passing a string pointer. The string WILL NOT be saved.
Line *newLine(char *text) {
	Line *allocatedLine = malloc(sizeof(Line));
	assert(allocatedLine != NULL);
	allocatedLine->next = NULL;
	allocatedLine->text = text;
	return allocatedLine;
}

// COMMANDS
Cmd *newCmd(CmdType type, int addr1, int addr2, Line *old, Line *new) {
	Cmd *allocatedCmd = malloc(sizeof(Cmd));
	assert(allocatedCmd != NULL);
	allocatedCmd->type = type;
	allocatedCmd->addr1 = addr1;
	allocatedCmd->addr2 = addr2;
	allocatedCmd->old = NULL;
	allocatedCmd->new = NULL;
	allocatedCmd->next = NULL;
	return allocatedCmd;
}

// DATA STRUCTURE
// A collection of functions that update the buffer

void insertLines() {

};
void updateLines() {

};
void deleteLines() {

};
void printLines() {

};


// CODE

void parseInput() {
	fgets(INPUT_STRING, MAX_INPUT_SIZE, stdin);
	char cmd = INPUT_STRING[strlen(INPUT_STRING) - 2];
	INPUT_STRING[strlen(INPUT_STRING) - 2] = '\0';
	if (cmd == 'u' || cmd == 'r') {
		int times = atoi(INPUT_STRING);
		assert(times >= 0);
		if (cmd == 'u') {
			undo(times);
		} else {
			redo(times);
		}
	} else if (cmd =='c' || cmd == 'd' || cmd == 'p') {
		char *token1 = strtok(INPUT_STRING, ",");
		char *token2 = strtok(NULL, ",");
		assert(token1 != NULL);
		assert(token2 != NULL);
		int addr1 = atoi(token1);
		int addr2 = atoi(token2);
		assert(addr1 >= 0);
		assert(addr2 >= 0);
		assert(addr1 <= addr2);
		if (cmd == 'c') {
			change(addr1, addr2);
		} else if (cmd == 'd') {
			delete(addr1, addr2);
		} else {
			print(addr1, addr2);
		}
	} else SHOULD_QUIT = true;
}

void change(int addr1, int addr2) {
	printf("> Change from %d to %d.\n", addr1, addr2);
	Line *newLineHead = NULL;
	Line *newLineCurr = NULL;
	for (int i = 0; i < (addr2 - addr1 + 1); i++) {
		// Get the new text line from stdin
		fgets(INPUT_STRING, MAX_INPUT_SIZE, stdin);
		// Remove \n from the string
		char *input = strtok(INPUT_STRING, "\n");
		assert(input != NULL);
		// Save the string in a Line
		Line *line = newLine(newText(input));
		if (newLineHead == NULL) {
			newLineHead = line;
			newLineCurr = line;
		} else {
			newLineCurr->next = line;
			newLineCurr = newLineCurr->next;
		}
	}
	if (newLineCurr != )
	// Move to addr1
	Line *currLine = BUFFER;
	for(int i = 1; i < addr1; i++) {
		currLine = currLine->next;
	}
	int lineIndex = 0;
	Line *oldLineHead = currLine;
	Line *oldLineCurr = currLine;
	while(currLine && lineIndex < (addr2-addr1+1)) {
		Line *line = newLine(oldLineCurr->text);
		if (oldLineHead == NULL) {
			oldLineHead = line;
			oldLineCurr = line;
		} else {
			oldLineCurr->next = line;
			oldLineCurr = oldLineCurr->next;
		}
		currLine = oldLineCurr->next;
		lineIndex++;

	}

	Cmd *cmd = newCmd(CHANGE, addr1, addr2, oldLineHead, newLineHead);
}

void delete(int addr1, int addr2) {
	printf("> Delete from %d to %d.\n", addr1, addr2);

}

void print(int addr1, int addr2) {
	printf("> Print from %d to %d.\n", addr1, addr2);
}

void undo(int times) {
	printf("> Undo %d times.\n", times);
	// FIRST OPTIMIZE AND CHECK FOR BOUNDS.
	/* Get the top cmd from UNDO_STACK and "undo" its action.
	 * Then, move it to the REDO_STACK. */
}

void redo(int times) {
	printf("> Redo %d times.\n", times);
	/* Get the top cmd from REDO_STACK and "redo" its action.
	 * Then, move it to the UNDO_STACK. */
}