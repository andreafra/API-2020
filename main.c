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
} Cmd;

// Global Variables
// User input
char inputStr[MAX_INPUT];

// 1 = parse Commands, 0 = handle text
int isCmdMode = 1;

// Parameters of a command
int addr1, addr2, times;

// the file buffer
Line *buffer = NULL;

// State of the editor
Line *currLine = NULL;
Line *prevLine = NULL;
int currLineIndex = 1;
int totalLines = 1;


// the queues to perform undo/redo
Cmd *undoStack = NULL;

// List: Lines of text

// Create a new Line
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

// Replace the text of a Line
void updateLine(Line *curr, Line *prev, char *text) {
    Line *newLine;
    // Create a new list
    if (prev == NULL && curr == NULL) {
        newLine = createLine(text);
        buffer = newLine;
        totalLines = 1;
        // Update prev line
        prevLine = newLine;
        currLine = NULL;
        return;
    }
    // Append an element to the list
    if (prev != NULL && curr == NULL) {
        newLine = createLine(text);
        prev->next = newLine;
        totalLines += 1;
        // Update prev line
        prevLine = newLine;
        currLine = NULL;
        return;
    }
    // else...
    // Replace an element in the list
    strcpy(curr->text, inputStr);
    // Update prev line
    prevLine = curr;
    currLine = curr->next;
}

// Delete a Line and link the previous Line with the next one
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
void insertLine(Line *prev, Line *line) {

}

// Print a Line text. Print '.' if the line is null.
void printLine(Line *line) {
    if (line)
        printf("%s", line->text);
    else
        printf(".");
}

// Stack: undo/redo

Cmd* push(Cmd* stack, CmdType type, int addr1, int addr2, Line *backup) {

}

Cmd* pop(Cmd* stack) {

};

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
//            change();
            break;
        case 'd': // Delete
            parseDoubleCmd();
            isCmdMode = 1;
            delete();
            break;
        case 'p': // Print
            parseDoubleCmd();
            isCmdMode = 1;
            print();
            break;
        case 'u': // Undo
            parseSingleCmd();
            undo();
            break;
        case 'r': // Redo
            parseSingleCmd();
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

void change() {
    if (addr1 > 0 && addr1 <= totalLines + 1) {
        // Start from the beginning (line 1)
        currLine = buffer;
        prevLine = NULL;
        // Move to addr1 line
        for (currLineIndex = 1; currLineIndex < addr1; currLineIndex++) {
            prevLine = currLine;
            currLine = currLine->next;
        }
        // Change each line till addr2 line
        for (int i = addr1; i <= addr2; i += 1) {
            // save input text in inputStr
            fgets(inputStr, MAX_INPUT, stdin);
            // updateLine also updates prevLine and currLine
            updateLine(currLine, prevLine, inputStr);
        }
        // Eat the final '.'
        fgets(inputStr, MAX_INPUT, stdin);
        isCmdMode = 1;
    }
}

void delete() {
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
            deleteLine(prevLine, currLine);
        }
    }
}

void print() {
    // Start from the beginning
    currLine = buffer;
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
                printLine(currLine);
                currLine = currLine->next;
            } else {
                printf(".\n");
            }
        }
    }
}

void undo() {

}

void redo() {

}