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

// Undo/Redo stack operations
void stack_push(Cmd **stack, int *size, Cmd *item) {
	if (*stack == NULL) {
		*stack = item;
	} else {
		item->next = *stack;
		*stack = item;
	}
	*size += 1;
}

Cmd *stack_pop(Cmd **stack, int *size) {
	if (*stack == NULL) return NULL;
	Cmd *top = *stack;
	*stack = top->next;
	*size -= 1;
	return top;
}

// DEBUG
void debug_printCmd(Cmd *cmd) {
	if (cmd == NULL) {
		printf("| <NULL>\n");
		return;
	}
	printf("+---------\n");
	printf("| Type: %s\n", cmd->type == CHANGE ? "CHANGE" : "DELETE");
	printf("| Addr1: %d\n", cmd->addr1);
	printf("| Addr2: %d\n", cmd->addr2);
	printf("| Old: ");
	Line* line = cmd->old;
	while (line) {
		printf("%s ", line->text);
		line = line->next;
	}
	printf("\n");
	printf("| New: ");
	line = cmd->new;
	while (line) {
		printf("%s ", line->text);
		line = line->next;
	}
	printf("\n+---------\n");
}

void debug_printList(Line *l) {
	Line *tmp = l;
	while(tmp) {
		printf("%s-", tmp->text);
		tmp = tmp->next;
	}
	printf("\n");
}

int debug_countList(Line *l) {
	Line *tmp = l;
	int i = 0;
	while(tmp) {
		i++;
		tmp = tmp->next;
	}
	return i;
}

// Commands from input

void change(int addr1, int addr2);
void delete(int addr1, int addr2);
void print(int addr1, int addr2);
void undo(int times);
void redo(int times);

int main() {
	//setbuf(stdin, NULL);
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

// Create a new Command
Cmd *newCmd(CmdType type, int addr1, int addr2, Line *old, Line *new) {
	Cmd *allocatedCmd = malloc(sizeof(Cmd));
	assert(allocatedCmd != NULL);
	allocatedCmd->type = type;
	allocatedCmd->addr1 = addr1;
	allocatedCmd->addr2 = addr2;
	allocatedCmd->old = old;
	allocatedCmd->new = new;
	allocatedCmd->next = NULL;
	return allocatedCmd;
}

// Get the last item in a List of Lines
Line *getLastLine(Line *list) {
	Line *curr = list;
	while(curr->next) {
		curr = curr->next;
	}
	return curr;
}

// DATA STRUCTURE
// A collection of functions that update the buffer

Line *goToLine(int number) {
	if (number < 1) return NULL;
	Line *curr = BUFFER;
	for (int i = 1; i < number && curr; i++) {
		curr = curr->next;
	}
	return curr;
}

// Append the passed list at the prev item.
// Example: If you add lines at 3, you append them at 2, and the old 3
// will be appended at the last item of the list.
void insertLines(Line *prev, Line *list) {
	Line *tmp;
	if (prev == NULL) {
		tmp = BUFFER;
		BUFFER = list;
		getLastLine(list)->next = tmp;
	} else {
		tmp = prev->next;
		prev->next = list;
		getLastLine(list)->next = tmp;
	}
}

void updateLines(Line *prev) {

}

// Returns the deleted lines as a list with the last item linked to NULL;
Line *deleteLines(Line *prev, Line *curr, int quantity) {
	if (prev == NULL && curr == NULL) return NULL;


	Line *tmp = curr;
	Line *next = NULL;
	for (int i = 0; i < quantity; i++) {
		if (tmp == NULL) {
			// If the list ends too soon it means that
			// we're at the end of the buffer, nothing
			// more to delete.
			break;
		} else if (i == quantity - 1) {
			// We're at the last item
			// we're unlinking it from the next.
			next = tmp->next;
			tmp->next = NULL;
			break;
		} else {
			// Go next
			tmp = tmp->next;
		}
	}
	if (prev == NULL && curr != NULL) {
		// We're editing the first line
		// [BUFFER] ->> NEXT
		BUFFER = next;
	} else {
		// PREV ->> NEXT
		prev->next = next;
	}

	return curr;
}

void printLines(Line *line, int quantity) {
	Line *tmp = line;
	for (int i = 0; i < quantity; i++) {
		if (tmp == NULL) {
			printf(".\n");
		} else {
			printf("%s\n", tmp->text);
			tmp = tmp->next;
		}
	}
}

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

Line *getInputLines(int quantity, Line **lastItem) {

	Line *list = NULL;
	*lastItem = NULL;

	for (int i = 0; i < quantity; i++) {
		// Get the new text line from stdin
		fgets(INPUT_STRING, MAX_INPUT_SIZE, stdin);
		// Remove \n from the string
		char *input = strtok(INPUT_STRING, "\n");
		assert(input != NULL);
		// Save the string in a Line
		Line *nLine = newLine(newText(input));

		if (list == NULL) {
			list = nLine;
			*lastItem = nLine;
		} else {
			(*lastItem)->next = nLine;
			*lastItem = nLine;
		}
	}
	// Chomp the '.'
	fgets(INPUT_STRING, MAX_INPUT_SIZE, stdin);

	return list;
}

void change(int addr1, int addr2) {
	// Get the new lines
	Line *lastNewLine = NULL;
	Line *newLines = getInputLines(addr2 - addr1 + 1, &lastNewLine);

	// REMOVE the old lines
	Line *prev = goToLine(addr1 - 1);
	Line *curr = prev ? prev->next : goToLine(addr1);
	Line *oldLines = deleteLines(prev, curr, addr2 - addr1 + 1);

	assert(debug_countList(newLines) >= debug_countList(oldLines));

	// Add Command to undo stack
	Cmd *cmd = newCmd(CHANGE, addr1, addr2, oldLines, newLines);
	stack_push(&UNDO_STACK, &UNDO_STACK_SIZE, cmd);

	// Substitute the old lines with the new ones
	insertLines(prev, newLines);
}

void delete(int addr1, int addr2) {
	if (addr1 == 0 && addr2 == 0) {
		Cmd *cmd = newCmd(DELETE, addr1, addr2, NULL, NULL);
		stack_push(&UNDO_STACK, &UNDO_STACK_SIZE, cmd);
		return;
	}

	// REMOVE the old lines
	Line *prev = goToLine(addr1 - 1);
	Line *curr = prev ? prev->next : goToLine(addr1);
	Line *oldLines = deleteLines(prev, curr, addr2 - addr1 + 1);

	// Add Command to undo stack
	Cmd *cmd = newCmd(DELETE, addr1, addr2, oldLines, NULL);
	stack_push(&UNDO_STACK, &UNDO_STACK_SIZE, cmd);
}

void print(int addr1, int addr2) {
	if (addr1 == 0 && addr2 == 0) {
		printf(".\n");
		return;
	}
	printLines(goToLine(addr1), addr2-addr1+1);
}

void undo(int times) {
	debug_printCmd(stack_pop(&UNDO_STACK, &UNDO_STACK_SIZE));
	// FIRST OPTIMIZE AND CHECK FOR BOUNDS.
	/* Get the top cmd from UNDO_STACK and "undo" its action.
	 * Then, move it to the REDO_STACK. */
}

void redo(int times) {
	/* Get the top cmd from REDO_STACK and "redo" its action.
	 * Then, move it to the UNDO_STACK. */
}