#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
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
	Line *oldText;	
	Line *newText;
	struct _Cmd *next;
} Cmd;

// The start of the file to write.
Line *BUFFER = NULL;

// Add a command to this stack when you DO/REDO a command.
Cmd *UNDO_STACK = NULL;
// Add a command to this stack when you UNDO a command.
Cmd *REDO_STACK = NULL;

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

// CODE

void parseInput() {
	fgets(INPUT_STRING, MAX_INPUT_SIZE, stdin);
	int inputLen = strnlen(INPUT_STRING, MAX_INPUT_SIZE);
	char cmd = INPUT_STRING[inputLen-1];

	assert( cmd == 'c' || cmd == 'd' || cmd == 'p' || cmd == 'u' || cmd == 'r');
	
	char* param1 = strtok(INPUT_STRING, ",");
}