#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

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
    Line *start_neighbour, *end_neighbour;
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

#define UNLINK_PAIR(p) {\
    p->start->prev = NULL;\
    p->end->next = NULL;\
}

#define MAX(a, b) (a > b ? a : b)

#define MIN(a, b) (a < b ? a : b)

#define CAP(x, cap) (x > cap ? cap : x)


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
void debug_printList(Line *l) {
    Line *start = NULL;
    Line *tmp = l;
    while (tmp && tmp != start) {
        start = l;
        printf("%s", tmp->text);
        tmp = tmp->next;
    }
    printf("\n");
}

void debug_printRange(Range *r) {
    if (r == NULL)
        printf("=> Range is Null\n");
    else if (r->start == NULL || r->end == NULL)
        printf("=> Range.start or Range.end is NULL\n");
    else
        printf("[ Start: %s | End: %s | Size: %d ]\n", r->start->text, r->end->text, r->size);
}

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
    if (cmd->old)
        debug_printRange(cmd->old);
    printf("\n");
    printf("| New: ");
    if (cmd->new)
        debug_printRange(cmd->new);
    printf("| Prev: %s\n", cmd->start_neighbour ? cmd->start_neighbour->text : "(null)");
    printf("| Next: %s\n", cmd->end_neighbour ? cmd->end_neighbour->text : "(null)");}

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

Cmd *createCmd(CmdType type, int addr1, int addr2, Range *old, Range *new, Line*, Line*);

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

void insert(int index, const Range *list);

Range *getRange(int i, int j);

int main() {
//    Line* p;
//    Line* q;
//
//    Range *a = getInputLines(1);
//	Range *b = getInputLines(1);
//	Range *c = getInputLines(1);
//	Range *d = getInputLines(1);
//    Range *e = getInputLines(1);
//
//    debug_printRange(get(1, 1, &p, &q));
//
//    insert(1, a);
//
//    debug_printRange(get(1, 1, &p, &q));
//
//
//    insert(2, b);
//
//    debug_printRange(get(1, 2, &p, &q));
//
//    insert(3, c);
//	insert(4, d);
//    insert(5, e);
//
//
//    debug_printList(BUFFER);
//
//	debug_printRange(get(1, 1, &p, &q));
//    debug_printRange(get(5, 5, &p, &q));
//    debug_printRange(get(1, 10, &p, &q));
//    debug_printRange(get(1, 4));
//    debug_printRange(get(1, 2));
//    debug_printRange(get(4, 5));

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
Cmd *createCmd(CmdType type, int addr1, int addr2, Range *old, Range *new, Line *start_neightbour, Line *end_neightbour) {
    Cmd *allocatedCmd = malloc(sizeof(Cmd));
    assert(allocatedCmd != NULL);
    allocatedCmd->type = type;
    allocatedCmd->addr1 = addr1;
    allocatedCmd->addr2 = addr2;
    allocatedCmd->old = old;
    allocatedCmd->new = new;
    allocatedCmd->start_neighbour = start_neightbour;
    allocatedCmd->end_neighbour = end_neightbour;
    allocatedCmd->next = NULL;
    return allocatedCmd;
}

void clearLineList(Line **l) {
    Line *tmp = *l;
    while(tmp) {
        free(tmp->text);
        Line *next = tmp->next;
        free(tmp);
        tmp = next;
    }
    *l = NULL;
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

// Insert an item in the buffer BEFORE the index-esim element.
// If list is NULL, return and do nothing.
void insert(const int index, const Range *list) {
    if (list == NULL || list->start == NULL || list->end == NULL) return;
	assert(index >= 1 && index <= BUFFER_SIZE + 1);
	if (BUFFER == NULL) { // like, who cares about the index
		assert(BUFFER_SIZE == 0);
		BUFFER = list->start;
		LINK(list->start, list->end)
	} else {
        Line *tmp = BUFFER;
        if (index > BUFFER_SIZE/2 + 1) { // add from tail
            int p = (BUFFER_SIZE + 1 - index);
            for (int i = 0; i < p; ++i)
                tmp = tmp->prev;
            Line* prev = tmp->prev;
            Line* next = tmp;

            LINK(prev, list->start);
            LINK(list->end, next);
	    } else { // add from head
            for (int i = 1; i < index; i++)
                tmp = tmp->next;
            Line* prev = tmp->prev;
            Line* next = tmp;

            LINK(prev, list->start);
            LINK(list->end, next);
	    }
	}
	if (index == 1) BUFFER = list->start; // same as adding at the end, but I need to update the BUFFER pointer
	BUFFER_SIZE += list->size;
}

Range *createRange(Line *start, Line *end, const int size) {
    Range *res = malloc(sizeof(Range));
    res->start = start;
    res->end = end;
    res->size = size;
    return res;
}

// Return a range containing the start and the end of a sub-list
Range *getRange(const int i, const int j) {
    assert(1 <= i && i <= j <= BUFFER_SIZE);
    if (BUFFER == NULL) {
        return NULL; // It should never get called tho
    }

    Line *start = BUFFER, *end = BUFFER;
    if (i > BUFFER_SIZE/2) {
        // Get j first, then backward to i
        for (int k = 0; k < BUFFER_SIZE - j + 1; k++) {
            end = end->prev;
        }
        start = end;
        for (int k = 0; k < j-i; k++) {
            start = start->prev;
        }
    } else {
        if (j-i < BUFFER_SIZE-j) {
            // Get i, then forward count to j
            for (int k = 1; k < i; k++) {
                start = start->next;
            }
            end = start;
            for (int k = 0; k < j-i; k++) {
                end = end->next;
            }
        } else {
            // Get i forward, and then j backward
            for (int k = 1; k < i; k++) {
                start = start->next;
            }
            for (int k = 0; k < BUFFER_SIZE - j + 1; k++) {
                end = end->prev;
            }
        }
    }
    return createRange(start, end, j-i+1);
}

void bufferAppend(Range *r, Line **prev, Line **next) {
    assert(r != NULL);
    if (BUFFER == NULL) {
        // Update buffer too
        BUFFER = r->start;
        LINK(r->end, r->start);
        BUFFER_SIZE = r->size;
        *prev = NULL;
        *next = NULL;
        return;
    } else {
        // Never update buffer here
        Line *end = BUFFER->prev;
        LINK(r->end, BUFFER);
        LINK(end, r->start);
        BUFFER_SIZE += r->size;
        *next = BUFFER;
        *prev = end;
    }
}

// Returns unlinked old range.
// Replaces old with new
Range *bufferChange(Range *old, Range *new, Line **prev, Line **next) {
    assert(old != NULL);
    if (BUFFER_SIZE == 1 || // I'm changing a single block
        (old->start->prev == old->end && old->end->next == old->start)) { // I'm updating everything
        // Update buffer
        BUFFER = new->start;
        LINK(new->end, new->start)
        *prev = NULL;
        *next = NULL;
    } else {
        Line *prev_ = old->start->prev;
        Line *next_ = old->end->next;
        LINK(prev_, new->start)
        LINK(new->end, next_)
        *prev = prev_;
        *next = next_;
    }
    UNLINK_PAIR(old);
    BUFFER_SIZE += new->size - old->size;
    return old;
}

Range *bufferDelete(Range *old, Line **prev, Line **next) {
    if ((old->start->prev == NULL && old->end->next == NULL) ||
        (old->start->prev == old->end && old->end->next == old->start)) {
        // Delete everything
        *prev = NULL;
        *next = NULL;
        UNLINK_PAIR(old)
        BUFFER_SIZE -= old->size;
        return old;
    } else {
        *prev = old->start->prev;
        *next = old->end->next;
        UNLINK_PAIR(old)
        LINK((*prev), (*next))
        BUFFER_SIZE -= old->size;
        return old;
    }
}

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
        // Save the string in a Line
        Line *nLine = createLine(createText(INPUT_STRING_BUFFER));
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

    // Make sure the ends of a range are fine
	assert(range->start->prev == NULL);
	assert(range->end->next == NULL);
    return range;
}

void change(int addr1, int addr2) {
    assert(0 <= addr1 && addr1 <= addr2);
    //applyUR();
    //clearRedoStack();

    Line *prev = NULL, *next = NULL;

    Range *old = NULL;
    Range *new = getInputLines(addr2-addr1+1);
    assert(new->start->prev == NULL && new->end->next == NULL);

    if (addr1 > BUFFER_SIZE) {
        //Only Append
        assert(addr1 == BUFFER_SIZE + 1);
        bufferAppend(new, &prev, &next);
    } else {
        if (addr2 <= BUFFER_SIZE) {
            // Only Change
            old = bufferChange(getRange(addr1, addr2), new, &prev, &next);
            if (addr1 == 1) BUFFER = new->start;
        } else {
            // Change and append
            old = bufferChange(getRange(addr1, CAP(addr2, BUFFER_SIZE)), new, &prev, &next);
        }
    }

    // Add Command to undo stack
    Cmd *cmd = createCmd(CHANGE, addr1, addr2, old, new, prev, next);
    stack_push(&UNDO_STACK, &UNDO_STACK_SIZE, cmd);

//    debug_printCmd(cmd);
}

void delete(int addr1, int addr2) {
    applyUR();
    clearRedoStack();
    assert(addr1 >= 0);
    Range *old = NULL;
    Line *prev = NULL, *next = NULL;

    if (!((addr1 == 0 && addr2 == 0) || (addr1 > BUFFER_SIZE))) {
        int end = CAP(addr2, BUFFER_SIZE);
        old = bufferDelete(getRange(addr1, end), &prev, &next);
        // Update buffer if I delete the first item
        if (addr1 == 1) BUFFER = next;
    }

    // Add Command to undo stack
    Cmd *cmd = createCmd(DELETE, addr1, addr2, old, NULL, prev, next);
    stack_push(&UNDO_STACK, &UNDO_STACK_SIZE, cmd);

}

void print(int addr1, int addr2) {
    applyUR();

    assert(addr1 >= 0);
    if (addr1 == 0 && addr2 == 0) {
        printf(".\n");
        return;
    }

    int i = addr1, j = addr2, dots = 0;

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
        Line *tmp = BUFFER;
        for (int k = 0; k < i-1; k++)
            tmp = tmp->next;
        for (; i <= j; i++) {
            printf("%s", tmp->text);
            tmp = tmp->next;
        }
    } else {
        dots = addr2 - addr1 + 1;
    }

    for (; dots > 0; dots--) {
        printf(".\n");
    }
}

void undo(int times) {
    int possibleUndo = UNDO_STACK_SIZE + UR;
    UR -= times > possibleUndo ? possibleUndo : times;

    assert(UR >= -UNDO_STACK_SIZE);
}

void redo(int times) {
    int possibleRedo = REDO_STACK_SIZE - UR;
    UR += times > possibleRedo ? possibleRedo : times;

    assert(UR <= REDO_STACK_SIZE);
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
    if (cmd->type == CHANGE) {
    
    } else { // DELETE

    }
    stack_push(&REDO_STACK, &REDO_STACK_SIZE, cmd);
}

void applyRedo() {
	assert(UR > 0 && UR <= REDO_STACK_SIZE);

    Cmd *cmd = stack_pop(&REDO_STACK, &REDO_STACK_SIZE);
    assert(cmd != NULL);
    if (cmd->type == CHANGE) {

    } else { // DELETE

    }
    stack_push(&UNDO_STACK, &UNDO_STACK_SIZE, cmd);
}
