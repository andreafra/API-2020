#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

 #define NDEBUG
#include <assert.h>

#define MAX_INPUT_SIZE 1024

typedef enum CmdType {
    CHANGE, DELETE
} CmdType;

typedef struct Line {
    char *text;
    struct Line *prev, *next;
} Line;

typedef struct Range {
	int size;
	Line *start, *end;
} Range;

typedef struct Cmd {
    CmdType type;
    int addr1, addr2;
    Range *old, *new;
    struct Cmd *next;
} Cmd;

#define LINK(a, b) {\
    a->next = b;\
    b->prev = a;\
    }

#define UNLINK(a, b) {\
	a->next = NULL;\
	b->prev = NULL;\
	}

// Counter for how many Undo/Redo remains to be applied
int UR = 0;
// Add a command to this stack when you DO/REDO a command.
Cmd *UNDO_STACK = NULL;
int UNDO_STACK_SIZE = 0;
// Add a command to this stack when you UNDO a command.
Cmd *REDO_STACK = NULL;
int REDO_STACK_SIZE = 0;

// Set it to true to terminate the program.
bool SHOULD_QUIT = false;

char INPUT_STRING_BUFFER[MAX_INPUT_SIZE];

Line *BUFFER = NULL;
int BUFFER_SIZE = 0;

// Read from stdin and handle commands.
void parseInput();

// Undo/Redo stack operations
void stack_push(Cmd **stack, int *size, Cmd *item) {
    item->next = *stack;
    *stack = item;
    *size += 1;
}

Cmd *stack_pop(Cmd **stack, int *size) {
    if (*stack == NULL) {
        assert(*size == 0);
        return NULL;
    }
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

    printf("\n");
    printf("| New: ");

    printf("\n+---------\n");
}

void debug_printList(Line *l) {
	Line *start = NULL;
	Line *tmp = l;
    while (tmp && tmp != start) {
    	start = l;
        printf("%s ", tmp->text);
        tmp = tmp->next;
    }
    printf("\n");
}

int debug_countList(Line *l) {
	Line *start = NULL;
    Line *tmp = l;
    int i = 0;
    while (tmp && tmp != start) {
    	start = l;
        i++;
        tmp = tmp->next;
    }
    return i;
}

char *createText(char *text);

Line *createLine(char *text);

Cmd *createCmd(CmdType type, int addr1, int addr2, Range *old, Range *new);

void clearRedoStack();

Range *getInputLines(int quantity);

void change(int addr1, int addr2);

void delete(int addr1, int addr2);

void print(int addr1, int addr2);

void undo(int times);

void redo(int times);

void applyUR();

void applyUndo();

void applyRedo();

void insert(const int index, const Range *list);

int main() {
	Range *a = getInputLines(1);
	Range *b = getInputLines(1);
	Range *c = getInputLines(1);
	Range *d = getInputLines(1);

	insert(1, a);
	insert(1, b);
	insert(2, c);
	insert(1, d);

	while (!SHOULD_QUIT) {
        // Each iteration is a command
        parseInput();
    }

    return 0;
}

// Save a string with the minimum size possible. Strings are immutable.
char *createText(char *text) {
    char *allocatedStr = malloc(strlen(text) * sizeof(char) + 1);
    assert(allocatedStr != NULL);
    strcpy(allocatedStr, text);
    return allocatedStr;
}

// Create a new Line by passing a string pointer. The string WILL NOT be saved.
Line *createLine(char *text) {
    Line *allocatedLine = malloc(sizeof(Line));
    assert(allocatedLine != NULL);
    allocatedLine->prev = NULL;
    allocatedLine->next = NULL;
    allocatedLine->text = text;
    return allocatedLine;
}

// Create a new Command
Cmd *createCmd(CmdType type, int addr1, int addr2, Range *old, Range *new) {
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

void clearLineList(Line *l) {
    Line *tmp = l;
    while(tmp) {
        free(tmp->text);
        Line *next = tmp->next;
        free(tmp);
        tmp = next;
    }
}

void clearRedoStack() {
    while(REDO_STACK) {
    	// TODO: Fix clearLineList
        // clearLineList(REDO_STACK->new);
        Cmd *next = REDO_STACK->next;
        free(REDO_STACK);
        REDO_STACK = next;
    }
    REDO_STACK = NULL;
    REDO_STACK_SIZE = 0;
}

// DATA STRUCTURE
void insert(const int index, const Range *list) {
	assert(index >= 1 && index <= size + 1);
	if (BUFFER == NULL) { // like, who cares about the index
		assert(BUFFER_SIZE == 0);
		BUFFER = list->start;
		LINK(list->start, list->end)
	} else {
		if (index < BUFFER_SIZE / 2) {
			// Insert from head
			Line *tmp = BUFFER;
			for (int i = 0; i < index-1; i++) {
				tmp = tmp->next;
				assert(tmp != NULL);
			}
			// Now tmp is where I want to add stuff
			Line *prev = tmp->prev;
			Line *next = tmp;
			LINK(prev, list->start)
			LINK(list->end, next)
		} else {
			// Insert from tail
			Line *tmp = BUFFER->prev;
			for (int i = 0; i < BUFFER_SIZE - index; i++) {
				tmp = tmp->prev;
			}
			// Now tmp is where I want to add stuff
			Line *next = tmp->next;
			Line *prev = tmp;
			LINK(prev, list->start)
			LINK(list->end, next)
		}
	}
	if (index == 1) BUFFER = list->start;
	BUFFER_SIZE += list->size;

	debug_printList(BUFFER);
}

Range *get(const int i, const int j) {

}

// INTRO CODE

void parseInput() {
    fgets(INPUT_STRING_BUFFER, MAX_INPUT_SIZE, stdin);
    char cmd = INPUT_STRING_BUFFER[strlen(INPUT_STRING_BUFFER) - 2];
	INPUT_STRING_BUFFER[strlen(INPUT_STRING_BUFFER) - 2] = '\0';
    if (cmd == 'u' || cmd == 'r') {
        int times = atoi(INPUT_STRING_BUFFER);
        assert(times >= 0);
        if (cmd == 'u') {
            undo(times);
        } else {
            redo(times);
        }
    } else if (cmd == 'c' || cmd == 'd' || cmd == 'p') {
        char *token1 = strtok(INPUT_STRING_BUFFER, ",");
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

Range *getInputLines(int quantity) {
    Range *range = calloc(1, sizeof(Range));
	range->size = quantity;

    for (int i = 0; i < quantity; i++) {
        // Get the new text line from stdin
        fgets(INPUT_STRING_BUFFER, MAX_INPUT_SIZE, stdin);
        // Remove \n from the string
        char *input = strtok(INPUT_STRING_BUFFER, "\n");
        assert(input != NULL);
        // Save the string in a Line
        Line *nLine = createLine(createText(input));
		if (range->start == NULL) { // first item
			range->start = nLine;
			range->end = nLine;
		} else { // second+ item, range->end is the prev item
			LINK(range->end, nLine)
			// Set last item as new item
			range->end = nLine;
		}
    }
    // Chomp the '.'
    fgets(INPUT_STRING_BUFFER, MAX_INPUT_SIZE, stdin);

	assert(range->start->prev == NULL);
	assert(range->end->next == NULL);
    return range;
}

void change(int addr1, int addr2) {
    applyUR();
    clearRedoStack();

    getInputLines(addr2-addr1+1);

    // Add Command to undo stack
    Cmd *cmd = createCmd(CHANGE, addr1, addr2, NULL, NULL);
    stack_push(&UNDO_STACK, &UNDO_STACK_SIZE, cmd);

}

void delete(int addr1, int addr2) {
    applyUR();
    clearRedoStack();

    assert(addr1 >= 0);
    Line *oldLines = NULL;

    unsigned i, j;

    if (addr1 <= BUFFER_SIZE) {
        if (addr1 == 0) {
            if (addr2 == 0) {
                i = 0;
                j = 0;
            } else {
                i = 0;
                j = addr2 - 1;
            }
        } else {
            if (addr2 <= BUFFER_SIZE) {
                i = addr1 - 1;
                j = addr2 - 1;
            } else {
                i = addr1 - 1;
                j = BUFFER_SIZE - 1;
            }
        }
        //oldLines = Report(BUFFER, i, j);
        //destroyTree(Delete(&BUFFER, i, j));
    } else {
        oldLines = NULL;
    }

    // Add Command to undo stack
    Cmd *cmd = createCmd(DELETE, addr1, addr2, oldLines, NULL);
    stack_push(&UNDO_STACK, &UNDO_STACK_SIZE, cmd);
}

void print(int addr1, int addr2) {
    applyUR();

    assert(addr1 >= 0);
    if (addr1 == 0 && addr2 == 0) {
        printf(".\n");
        return;
    }

    unsigned i = addr1, j = addr2, dots = 0;

    if (i == 0 && j > 0) {
        i = 1;
        printf(".\n");
    }
    if (i <= BUFFER_SIZE) {
        if (j > BUFFER_SIZE) {
            j = BUFFER_SIZE;
            dots = addr2 - BUFFER_SIZE;
        } else { // i and j exists
            dots = 0;
        }
    } else {
        dots = addr2 - addr1 + 1;
    }

    /*Line *lines = Report(BUFFER, i-1, j-1); // dealloc?
    Line *tmp = lines;
    while (tmp) {
        printf("%s\n", tmp->text);
        Line *next = tmp->next;
        free(tmp);
        tmp = next;
    }*/
    for (; dots > 0; dots--) {
        printf(".\n");
    }
}

void undo(int times) {
    int possibleUndo = UNDO_STACK_SIZE + UR;
    UR -= times > possibleUndo ? possibleUndo : times;

    assert(UR >= -UNDO_STACK_SIZE);

//    int initialStackSize = UNDO_STACK_SIZE;
//    for (int i = 0; i < (times > initialStackSize ? initialStackSize : times); i++) applyUndo();
}

void redo(int times) {
    int possibleRedo = REDO_STACK_SIZE - UR;
    UR += times > possibleRedo ? possibleRedo : times;

    assert(UR <= REDO_STACK_SIZE);

//    int initialStackSize = REDO_STACK_SIZE;
//    for (int i = 0; i < (times > initialStackSize ? initialStackSize : times); i++) applyRedo();
}

void applyUR() {
    if (UR > 0) {
        for ( ; UR > 0; UR--) applyRedo();
    }
    else if (UR < 0) {
        for ( ; UR < 0; UR++) applyUndo();
    }
    // Reset UR
    UR = 0;
}

void applyUndo() {
	assert(UR >= -UNDO_STACK_SIZE && UR < 0);

    Cmd *cmd = stack_pop(&UNDO_STACK, &UNDO_STACK_SIZE);
    assert(cmd != NULL);
    /*if (cmd->type == CHANGE) {
        // UNDO: Remove lines from addr1 to addr2
        //       and add at addr1 the old lines
        destroyTree(Delete(&BUFFER, cmd->addr1 - 1, cmd->addr2 - 1));
        Line *tmp = cmd->old;
        unsigned index = cmd->addr1 - 1;
        while (tmp) {
            Insert(&BUFFER, index, tmp->text);
            index++;
            tmp = tmp->next;
        }
    } else { // DELETE
        unsigned addr, offset = 0;
        if (cmd->addr2 == 0) {
            // do nothing
        } else {
            if (cmd->addr1 == 0) addr = 0;
            else addr = cmd->addr1 - 1;
            Line *tmp = cmd->old;
            while (tmp) {
                Insert(&BUFFER, addr + offset, tmp->text);
                offset++;
                tmp = tmp->next;
            }
        }
    }*/
    stack_push(&REDO_STACK, &REDO_STACK_SIZE, cmd);
}

void applyRedo() {
	assert(UR > 0 && UR <= REDO_STACK_SIZE);

    Cmd *cmd = stack_pop(&REDO_STACK, &REDO_STACK_SIZE);
    assert(cmd != NULL);
  /*  if (cmd->type == CHANGE) {
        assert(cmd->addr1 != 0);
        if (cmd->old != NULL) {
            destroyTree(Delete(&BUFFER,
                               cmd->addr1 - 1,
                               (cmd->addr2 > BUFFER_SIZE ? BUFFER_SIZE : cmd->addr2) - 1));
        }
        Line *tmp = cmd->new;
        unsigned index = cmd->addr1 - 1;
        while (tmp) {
            Insert(&BUFFER, index, tmp->text);
            index++;
            tmp = tmp->next;
        }
    } else { // DELETE
        unsigned addr1 = cmd->addr1, addr2 = cmd->addr2, i, j;

        if (addr1 <= BUFFER_SIZE) {
            if (addr1 == 0) {
                if (addr2 == 0) {
                    i = 0;
                    j = 0;
                } else {
                    i = 0;
                    j = addr2 - 1;
                }
            } else {
                if (addr2 <= BUFFER_SIZE) {
                    i = addr1 - 1;
                    j = addr2 - 1;
                } else {
                    i = addr1 - 1;
                    j = BUFFER_SIZE - 1;
                }
            }
            destroyTree(Delete(&BUFFER, i, j));
        }
    }*/
    stack_push(&UNDO_STACK, &UNDO_STACK_SIZE, cmd);
}
