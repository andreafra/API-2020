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
	char cmd = INPUT_STRING[strlen(INPUT_STRING) - 2];
	INPUT_STRING[strlen(INPUT_STRING) - 2] = '\0';
	if (cmd == 'u' || cmd == 'r') {
		int times = atoi(INPUT_STRING);
		assert(times >= 0);
		printf(">%d%c\n", times, cmd);
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
		printf(">%d,%d%c\n", addr1, addr2, cmd);
		if (cmd == 'c') {
			change(addr1, addr2);
		} else if (cmd == 'd') {
			delete(addr1, addr2);
		} else {
			print(addr1, addr2);
		}
	} else SHOULD_QUIT = true;
}
