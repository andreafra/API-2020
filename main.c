#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_INPUT 1024

// Prototypes
void parseCmd();

void parseSingleCmd();

void parseDoubleCmd();

void change();

void delete();

void print();

void undo();

void redo();

void performUR(int);

// Data structures
typedef enum command_type_ {
	CHANGE,
	DELETE
} CmdType;

// A list element representing a row of text
// To be used in a single-linked list
typedef struct line_ {
	char *text;
	struct line_ *next;
} Line;

// A list element representing a command
// To be used in a double-linked list
typedef struct cmd_ {
	CmdType type;
	int addr1;
	int addr2;
	struct cmd_ *prev;
	struct cmd_ *next;
	Line *newText;
	Line *oldText;
} Cmd;

// Global Variables
// User input
char inputStr[MAX_INPUT];

// 1 = parse Commands, 0 = handle text
int isCmdMode = 1;

// Negative is Undo, Positive is Redo
int numberOfUR = 0;
// Bool, true if last command was either Undo or Redo
int lastCmdWasUR = 0;

// Parameters of a command
int addr1, addr2, times;

// the file buffer
Line *buffer = NULL;

// State of the editor
Line *currLine = NULL;
Line *prevLine = NULL;
int currLineIndex = 1;
int totalLines = 1;


// the stacks to perform undo/redo
Cmd *undoStack = NULL;
Cmd *undoStackTop = NULL;

Cmd *redoStack = NULL;
Cmd *redoStackTop = NULL;

// List: Lines of text

// Create a new Line
// Returns a pointer to the new line
Line *createLine(char *text) {
	Line *newLine = malloc(sizeof(Line));
	char *newText = malloc(sizeof(char) * MAX_INPUT);
	if (newLine && newText) {
		strcpy(newText, text);
		newLine->next = NULL;
		newLine->text = newText;
		return newLine;
	}
	// Print error
	printf("ERROR: Can't allocate a new Line!");
	exit(12);
}

// Count lines in linked list
int countLines(Line *l) {
	int i = 0;
	Line *curr = l;
	while(curr) {
		i += 1;
		curr = curr->next;
	}
	return i;
}

// Replace the text of a Line
// It can:
// 1. Create a new buffer
// 2. Append a line to the buffer
// 3. Replace a line in the buffer
// At the end, it moves the line pointer forward by one position.
void updateLine(Line *curr, Line *prev, char *text) {
	Line *newLine;
	// 1. Create a new list
	if (prev == NULL && curr == NULL) {
		newLine = createLine(text);
		buffer = newLine;
		// GLOBAL: Update line count
		totalLines = 1;
		// GLOBAL: Update prev line
		prevLine = newLine;
		currLine = NULL;
		return;
	}
	// 2. Append an element to the list
	if (prev != NULL && curr == NULL) {
		newLine = createLine(text);
		prev->next = newLine;
		// GLOBAL: Update line count
		totalLines += 1;
		// GLOBAL: Update prev line
		prevLine = newLine;
		currLine = NULL;
		return;
	}
	// ELSE:
	// 3. Replace an element in the list
	// (overwrite the already allocated string)
	strcpy(curr->text, inputStr);
	// GLOBAL: Update prev line and this line pointer
	prevLine = curr;
	currLine = curr->next;
}

// Delete a Line (line) and link the previous (prev) Line with the next one
void deleteLine(Line *prev, Line *line) {
	// delete a line only if it exists
	if (line) {
		Line *next = line->next;
		// Free memory
		free(line->text);
		free(line);
		if (prev) {
			// it wasn't the first element of the list
			// link together the prev and the next element
			prev->next = next;
			currLine = next;
			prevLine = prev;
		} else {
			// it was the first element of the list
			buffer = next;
			currLine = next;
			prevLine = NULL;
		}
		totalLines -= 1;
	}
}

// Insert a Line between prev and prev->next
// At the end, set the currLine to next
void insertLine(Line *prev, char *text) {
	Line *next = prev->next; // can be NULL
	Line *newLine = createLine(text);
	if (prev)
		prev->next = newLine;
	else
		buffer = newLine;
	// Update refs
	newLine->next = next;
	// GLOBAL: Update line count
	totalLines += 1;
	// GLOBAL: Update prev line
	prevLine = newLine;
	currLine = next;
	return;
}

// Stack: undo/redo

// Create a new Command
// Returns a pointer to the new command
Cmd *createCmd(CmdType type, int addr1, int addr2, Line *newText, Line *oldText) {
	Cmd *newCmd = malloc(sizeof(Cmd));
	if (newCmd) {
		newCmd->type = type;
		newCmd->prev = NULL;
		newCmd->next = NULL;
		newCmd->addr1 = addr1;
		newCmd->addr2 = addr2;
		newCmd->newText = newText;
		newCmd->oldText = oldText;
		return newCmd;
	}
	// Print error
	printf("ERROR: Can't allocate a new Cmd!");
	exit(12);
}

// Append text in reverse:
// a, b, c --> c, b, a
void appendOldTextToCmd(Cmd *cmd, char *text) {
	if (text) {
		Line *newLine = createLine(text);
		if(cmd->oldText == NULL) {
			cmd->oldText = newLine;
		} else {
			// TODO: Optimize to O(1) instead of O(n)
			Line *lastLine = cmd->oldText;
			while(lastLine->next) {
				lastLine = lastLine->next;
			}
			lastLine->next = newLine; // Append at the end
		}
	}
}

// Append in reverse:
// a, b, c --> c, b, a
void appendNewTextToCmd(Cmd *cmd, char *text) {
	if (text) {
		Line *newLine = createLine(text);
		if(cmd->newText == NULL) {
			cmd->newText = newLine;
		} else {
			// TODO: Optimize to O(1) instead of O(n)
			Line *lastLine = cmd->newText;
			while(lastLine->next) {
				lastLine = lastLine->next;
			}
			lastLine->next = newLine; // Append at the end
		}
	}
}

// Free a Cmd and all of its lists
void freeCmd(Cmd *cmd) {
	if (cmd) {
		while (cmd->oldText) {
			Line *next = cmd->oldText->next;
			free(cmd->oldText->text);
			cmd->oldText = next;
		}
		while (cmd->newText) {
			Line *next = cmd->newText->next;
			free(cmd->newText->text);
			cmd->newText = next;
		}
		free(cmd);
	}
}

Cmd *popUndo() {
	if (undoStackTop == NULL) {
		return NULL;
	} else {
		Cmd *poppedItem = undoStackTop;
		if (undoStackTop->prev == NULL)
			undoStack = NULL;
		undoStackTop = undoStackTop->prev;

		return poppedItem;
	}
}

void *pushUndo(Cmd *cmd) {
	if (undoStack == NULL) {
		undoStack = cmd;
	} else {
		undoStackTop->next = cmd;
		cmd->prev = undoStackTop;
	}
	undoStackTop = cmd;
}

Cmd *popRedo() {
	if (redoStackTop == NULL) {
		return NULL;
	} else {
		Cmd *poppedItem = redoStackTop;
		if (redoStackTop->prev == NULL)
			undoStack = NULL;
		redoStackTop = redoStackTop->prev;

		return redoStackTop;
	}
}

void *pushRedo(Cmd *cmd) {
	if (redoStack == NULL) {
		redoStack = cmd;
	} else {
		redoStackTop->next = cmd;
		cmd->prev = redoStackTop;
	}
	redoStackTop = cmd;
}

void cleanRedo() {
	while(redoStack) {
		Cmd *next = redoStack->next;
		freeCmd(redoStack);
		redoStack = next;
	}
	
	redoStack = NULL;
	redoStackTop = NULL;
}

// Entrypoint
int main() {
	do {
		if (isCmdMode)
			parseCmd();
		else
			change();
	} while (1);

	return 0;
}

void parseCmd() {
	// I need to parse the command
	// Read stdin
	fgets(inputStr, MAX_INPUT, stdin);

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
			isCmdMode = 0;
			performUR(1);
			break;
		case 'd': // Delete
			parseDoubleCmd();
			isCmdMode = 1;
			performUR(1);
			delete();
			break;
		case 'p': // Print
			parseDoubleCmd();
			isCmdMode = 1;
			performUR(0);
			print();
			break;
		case 'u': // Undo
			parseSingleCmd();
			lastCmdWasUR = 1;
			undo();
			break;
		case 'r': // Redo
			parseSingleCmd();
			lastCmdWasUR = 1;
			redo();
			break;
		case 'q':
			exit(0);
		default:
			break;
	}
}

// If you're getting weird values and need fixes, check 
// https://stackoverflow.com/questions/5493235
void parseSingleCmd() {
	times = strtol(inputStr, NULL, 10);
}

void parseDoubleCmd() {
	char *token1 = strtok(inputStr, ",");
	char *token2 = strtok(NULL, ",");
	addr1 = strtol(token1, NULL, 10);
	addr2 = strtol(token2, NULL, 10);
}

// It is not possible to change lines that don't exist, unless it's the first one
// after the end of the current buffer.
void change() {
	if (addr1 > 0 && addr1 <= totalLines + 1) {
		// Create new command
		Cmd *newCmd = createCmd(CHANGE, addr1, addr2, NULL, NULL);

		// Start from the beginning (line 1)
		prevLine = NULL;
		currLine = buffer;
		// Move to addr1 line
		for (currLineIndex = 1; currLineIndex < addr1; currLineIndex++) {
			prevLine = currLine;
			currLine = currLine->next;
		}
		// Change each line till addr2 line
		for (int i = addr1; i <= addr2; i += 1) {
			// save input text in inputStr
			fgets(inputStr, MAX_INPUT, stdin);
			// filter out the '\n' at the end
			char *inputStrNoNewLine = strtok(inputStr, "\n");
			// updateLine also updates prevLine and currLine
			// backup this line!
			if (currLine) 
				appendOldTextToCmd(newCmd, currLine->text);
			appendNewTextToCmd(newCmd, inputStrNoNewLine);

			// Update and move forward
			updateLine(currLine, prevLine, inputStrNoNewLine);
		}
		// Eat the final '.'
		fgets(inputStr, MAX_INPUT, stdin);
		// Reset to command mode
		isCmdMode = 1;

		// Save command
		pushUndo(newCmd);
	}
}

// It is possible to delete lines that don't exist.
void delete() {
	// Create new command
	Cmd *newCmd = createCmd(DELETE, addr1, addr2, NULL, NULL);


	if (addr1 > 0 && addr1 <= totalLines + 1) {
		// Start from the beginning (line 1)
		currLine = buffer;
		prevLine = NULL;
		// Move to addr1 line
		for (currLineIndex = 1; currLineIndex < addr1; currLineIndex++) {
			prevLine = currLine;
			currLine = currLine->next;
		}
		// Delete each line till addr2 line
		for (int i = addr1; i <= addr2; i += 1) {
			// backup this line!
			if (currLine) 
				appendOldTextToCmd(newCmd, currLine->text);
			
			deleteLine(prevLine, currLine);
		}
	}
	// Save command
	pushUndo(newCmd);
}

void print() {
	// Start from the beginning
	currLine = buffer;
	prevLine = NULL;

	for ( ; addr1 < (addr2 < 1 ? addr2+1 : 1); addr1++) {
		printf(".\n");
	}
	// Move to addr1 line
	for (currLineIndex = 1; currLineIndex < addr1; currLineIndex++) {
		currLine = currLine->next;
	}
	// Print each line till addr2 line
	if (addr1 >= 1) {
		for (int i = addr1; i <= addr2; i += 1) {
			if (currLine != NULL) {
				printf("%s\n", currLine->text);                
				currLine = currLine->next;
			} else {
				printf(".\n");
			}
		}
	}
}

void undo() {
	numberOfUR -= times;
}

void redo() {
	numberOfUR += times;
}

void printCmd(Cmd *cmd) {
	printf("| Type: %s\n", cmd->type == CHANGE ? "CHANGE" : "DELETE");
	printf("| Addr1: %d\n", cmd->addr1);
	printf("| Addr2: %d\n", cmd->addr2);
	printf("| OldText: ");
	Line* line = cmd->oldText;
	while (line) {
		printf("%s ", line->text);
		line = line->next;
	}
	printf("\n");
	printf("| NewText: ");
	line = cmd->newText;
	while (line) {
		printf("%s ", line->text);
		line = line->next;
	}
	printf("\n");
}

// This function is called whenever a non-undo, non-redo command is called,
// therefore applying all pending undo/redo. This allows us to condensate changes,
// and only apply them once.
void performUR(int allowRedoWipe) {
	// DEBUG: DeltaUR
	// printf("[%d]\n", numberOfUR);
	if (numberOfUR > 0) {
		// REDO

	}
	if (numberOfUR < 0) {
		// UNDO n times
		for (int i = 0; i < -numberOfUR; i++) {
			// get last undo
			Cmd *lastCmd = popUndo();
			if (lastCmd) {
				// DEBUG: Cmd
				// printCmd(lastCmd);
				
				if (lastCmd->type == CHANGE) {
					// change / delete lines
					/*
						1. Get at the oldtest item of oldText and get size of oldText
						2. (Addr2-Addr1+1) - #oldText = num of lines to delete
						3. At Addr1, insert #oldText in order
						4. When I get at the end of oldText, delete each line to delete    
					*/
					int numOfOldLines = countLines(lastCmd->oldText);
					int linesToDelete = (lastCmd->addr2 - lastCmd->addr1 + 1) - numOfOldLines;
					
					// Get to addr1
					currLine = buffer;
					prevLine = NULL;
					// Move to addr1 line
					for (currLineIndex = 1; currLineIndex < lastCmd->addr1; currLineIndex++) {
						prevLine = currLine;
						currLine = currLine->next;
					}
					// Change each line till addr2 line
					Line *oldLine = lastCmd->oldText;
					while(oldLine) {
						// Ad-hoc reimplement
						strcpy(currLine->text, oldLine->text);
						// Go to next line
						prevLine = currLine;
						currLine = currLine->next;
						// updateLine(currLine, prevLine, oldLine->text);
						oldLine = oldLine->next;
					}
					// Delete all remaining lines
					// At this point, I should be at Addr2 + 1
					for (int i = 0; i < linesToDelete; i += 1) {
						deleteLine(prevLine, currLine);
					}

				} else if (lastCmd->type == DELETE) {
					// insert lines
					/*
						1. Get at the oldtest item of oldText and get size of oldText
						2. Insert Lines from Addr1 to Addr2 and hopefully empty oldText
					*/
					// Get to addr1
					currLine = buffer;
					prevLine = NULL;
					// Move to addr1 line
					for (currLineIndex = 1; currLineIndex < lastCmd->addr1; currLineIndex++) {
						prevLine = currLine;
						currLine = currLine->next;
					}
					// Change each line till addr2 line
					Line *oldLine = lastCmd->oldText;
					while(oldLine) {
						// Ad-hoc insert?
						Line *newLine = createLine(oldLine->text);
						if (currLineIndex == 1) {
							newLine->next = buffer;
							buffer = newLine;
						} else {
							Line *next = prevLine->next;
							prevLine->next = newLine;
							newLine->next = next;
						}
						// Go to next line
						prevLine = newLine;
						currLine = newLine->next;

						oldLine = oldLine->next;
					}
				}
			}
		}
	}
	if (allowRedoWipe) {
		cleanRedo();
	}
	// else do nothing since the number of redos matches the number of undos.
	// Reset
	numberOfUR = 0;
}
