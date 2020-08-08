#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

// #define NDEBUG
#include <assert.h>

#define MAX_INPUT_SIZE 1024

typedef enum CmdType_ { CHANGE, DELETE } CmdType;

typedef struct Line_ {
	char *text;
	struct Line_ *next;
} Line;

typedef struct Cmd_ {
	CmdType type;
	int addr1;
	int addr2;
	Line *old;
	Line *new;
	struct Cmd_ *next;
} Cmd;

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

typedef struct Node_ {
	char *value;
	struct Node_ *parent, *left, *right;
	unsigned size;
} Node;

typedef struct Tree_ {
	Node *root;
	unsigned size;
} Tree;

char **inOrder(Tree *tree);

// DEBUG
__unused void debug_printCmd(Cmd *cmd) {
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

__unused int debug_countList(Line *l) {
	Line *tmp = l;
	int i = 0;
	while(tmp) {
		i++;
		tmp = tmp->next;
	}
	return i;
}

__unused void debug_printTree(Tree *tree) {
	if (tree) {
		char **sstr = inOrder(tree);
		for (unsigned xx = 0; xx < tree->size; xx++) {
			printf("%s ", sstr[xx]);
		}
		printf("\n");
	} else printf("<null>\n");
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

// DATA STRUCTURE

static inline Node *createNode(char *value) {
	Node *node = malloc(sizeof(Node));
	node->value = value;
	node->size = 1;
	node->parent = NULL;
	node->left = NULL;
	node->right = NULL;
	return node;
}

static inline Tree *createTree() {
	Tree *tree = malloc(sizeof(Tree));
	tree->root = NULL;
	tree->size = 0;
	return tree;
}

void postOrderFree_r(Node *n) {
	if (n == NULL) return;
	postOrderFree_r(n->left);
	postOrderFree_r(n->right);
	free(n);
	//free(n->value);
}

void destroyTree(Tree *tree) {
	if (tree == NULL) return;
	else if (tree->root == NULL) {
		free(tree);
		return;
	}
	postOrderFree_r(tree->root);
	free(tree);
}

/* Iterative in-order traversal.
 * Takes a tree* as input, and returns a string (a pointer to char).
 * It's faster to return (copy) one pointer than the whole string.
 * It could print nodes directly as it traverses the tree (and return void),
 * but that would mean calling putchar() or printf("%c") a large number of times,
 * instead of "appending" to the array result. */
char **inOrder(Tree *tree) {
	Node *current = tree->root;

	char **result = calloc(tree->size, sizeof(*result)); // Slower
	// static char *result[S_MAX_LEN]; // Faster

	unsigned i = 0;
	if (current == NULL)
		return result;
	Node **stack = malloc(tree->size * sizeof(**stack));
	size_t stackIndex = 0;

	while (true) {
		while (current) {
			stack[stackIndex] = current;
			stackIndex++;
			current = current->left;
		}
		if (stackIndex) {
			stackIndex--;
			current = stack[stackIndex];
			result[i] = current->value;
			i++;
			current = current->right;
		}
		else
			break;
	}
	free(stack);
	return result;
}

// Pointer to a tree, and a pointer to its node object that we want to rotate right.
static void rotateRight(Tree *tree, Node *node) {
	Node *parent = node->parent;
	Node *Y = node->left;

	if (Y == NULL) return; // we can't rotate the node with nothing!

	Node *B = Y->right;
	Y->parent = parent;
	if (parent) {
		if (node == parent->left)                               // node is left child
			parent->left = Y;
		else                                                    // node is right child
			parent->right = Y;
	}
	else
		tree->root = Y;

	node->parent = Y;
	Y->right = node;
	if (B)
		B->parent = node;
	node->left = B;

	node->size = (node->left ? node->left->size : 0) + (node->right ? node->right->size : 0) + 1;
	Y->size = (Y->left ? Y->left->size : 0) + (Y->right ? Y->right->size : 0) + 1;
}

// Input: Pointer to a tree, and a pointer to its node object that we want to rotate left.
static void rotateLeft(Tree *tree, Node *node) {
	Node *parent = node->parent;
	Node *X = node->right;
	if (!X)
		return; // we can't rotate the node with nothing!
	Node *B = X->left;
	X->parent = parent;
	if (parent) {
		if (node == parent->left) // node is left child
			parent->left = X;
		else // node is right child
			parent->right = X;
	}
	else
		tree->root = X;

	node->parent = X;
	X->left = node;
	if (B)
		B->parent = node;
	node->right = B;

	node->size = (node->left ? node->left->size : 0) + (node->right ? node->right->size : 0) + 1;
	X->size = (X->left ? X->left->size : 0) + (X->right ? X->right->size : 0) + 1;
}

// Splays node to the top of the tree, making it new root of the tree.
// Input: Pointer to a tree, and a pointer to its node object that we want to splay to the root.
void splay(Tree *tree, Node *node) {
	if (node == NULL) return;

	Node *parent = node->parent;

	while (parent) {
		Node *grandParent = parent->parent;

		if (!grandParent) {
			/* Zig */
			if (node == parent->left)
				rotateRight(tree, parent);
			else
				rotateLeft(tree, parent);
		}

		else if (node == parent->left) {
			if (parent == grandParent->left) {
				/* Zig-zig */
				rotateRight(tree, grandParent);
				rotateRight(tree, parent);
			}
			else {
				/* Zig-zag - if (parent == grandParent->right) */
				rotateRight(tree, parent);
				rotateLeft(tree, grandParent);
			}
		}

		else if (node == parent->right) {
			if (parent == grandParent->right) {
				/* Zig-zig */
				rotateLeft(tree, grandParent);
				rotateLeft(tree, parent);
			}
			else {
				/* Zig-zag (parent == grandParent.left) */
				rotateLeft(tree, parent);
				rotateRight(tree, grandParent);
			}
		}

		parent = node->parent;
	}
}


// METHODS TO USE

/* Input: Integer number k - the rank of a node (0 <= k < size of the whole tree).
 * Output: The k-esim smallest element in the tree (a node object). Counting starts from 0.
 * This is a public method, which splays the found node to the top of the tree. */
Node *Index(Tree *tree, unsigned k) {
	Node *node = tree->root;
	while (node) {
		Node *left = node->left;
		Node *right = node->right;
		unsigned s = left ? left->size : 0;
		if (k == s)
			break;
		else if (k < s) {
			if (left) {
				node = left;
				continue;
			}
			break;
		}
		else {
			if (right) {
				k = k - s - 1;
				node = right;
				continue;
			}
			break;
		}
	}
	splay(tree, node);
	return node;
}

static Node *subtreeMaximum(Tree *tree, Node *node) {
	if (!node)
		return NULL;
	while (node->right)
		node = node->right;
	splay(tree, node);
	return node;
}

// Merges two Splay trees, tree1 and tree2, using the last element (of highest rank)
// in tree1 (left string) as the node for merging, into a new tree.
static Tree *Concat(Tree *tree1, Tree *tree2) {
	if (tree1 == NULL || tree1->root == NULL)
		return tree2;
	if (tree2 == NULL || tree2->root == NULL)
		return tree1;
	Node *root2 = tree2->root;
	Node *root1 = subtreeMaximum(tree1, tree1->root);
	root2->parent = root1;
	root1->right = root2;
	root1->size = (root1->left ? root1->left->size : 0) + (root1->right ? root1->right->size : 0) + 1;
	tree1->size = root1->size;
	return tree1;
}

// Splits the Tree into two trees at the rank-esim element from the left.
// A B C D split at 1 is (A B) (C D).
static void Split(Tree *tree, unsigned rank, Tree **tree1, Tree **tree2) {
	Node *root1 = Index(tree, rank);
	Node *root2 = root1->right;
	root1->right = NULL;
	root1->size = (root1->left ? root1->left->size : 0) + (root1->right ? root1->right->size : 0) + 1;
	*tree1 = createTree();
	(*tree1)->root = root1;
	(*tree1)->size = root1->size;
	*tree2 = createTree();
	if (root2) {
		root2->parent = NULL;
		(*tree2)->root = root2;
		(*tree2)->size = root2->size;
	}
}

static Tree* Delete(Tree **tree, unsigned i, unsigned j) {
	Tree *left = NULL, *middle = NULL, *right = NULL;
	Split(*tree, j, &middle, &right);
	if (i > 0)
		Split(middle, i-1, &left, &middle);

	*tree = Concat(left, right);
	return middle;
}

static void Insert(Tree **tree, unsigned rank, char *value) {
	Node *node = createNode(value);
	Tree *node_tree = createTree();
	node_tree->root = node;
	node_tree->size = 1;
	Tree *left = NULL, *right = *tree;
	if (rank > 0)
		Split(*tree, rank-1, &left, &right);

	*tree = Concat(Concat(left, node_tree), right);
}


typedef struct List_ {
	Node *value;
	struct List_ *next;
} List;

List *createListItem(Node *node) {
	List *l = malloc(sizeof(List));
	assert(l != NULL);
	l->value = node;
	l->next = NULL;
	return l;
}

void listPush(List **l, Node *n) {
	static List *tail = NULL;
	List *newItem = createListItem(n);
	if (*l != NULL) // List already exists
		tail->next = newItem;
	else // List doesn't exists
		*l = newItem;
	tail = newItem;
}

List *Report(Tree *tree, unsigned i, unsigned j) {
	assert(i >= 0);
	assert(i <= j);
	List *l = NULL;
	for (unsigned t = i; t <= j; t++) {
		Node *n = Index(tree, t);
		assert(n != NULL);
		listPush(&l, n);
	}

	return l;
}

// INTRO CODE

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
	// TODO: Make a new function

	assert(debug_countList(newLines) >= debug_countList(oldLines));

	// Add Command to undo stack
	Cmd *cmd = newCmd(CHANGE, addr1, addr2, oldLines, newLines);
	stack_push(&UNDO_STACK, &UNDO_STACK_SIZE, cmd);

	// Substitute the old lines with the new ones

}

void delete(int addr1, int addr2) {
	if (addr1 == 0 && addr2 == 0) {
		Cmd *cmd = newCmd(DELETE, addr1, addr2, NULL, NULL);
		stack_push(&UNDO_STACK, &UNDO_STACK_SIZE, cmd);
		return;
	}

	// TODO: REMOVE the old lines

	// Add Command to undo stack
	Cmd *cmd = newCmd(DELETE, addr1, addr2, oldLines, NULL);
	stack_push(&UNDO_STACK, &UNDO_STACK_SIZE, cmd);
}

void print(int addr1, int addr2) {
	if (addr1 == 0 && addr2 == 0) {
		printf(".\n");
		return;
	}
	// TODO: Fetch and print lines
}

void undo(int times) {
	debug_printCmd(stack_pop(&UNDO_STACK, &UNDO_STACK_SIZE));
	// FIRST OPTIMIZE AND CHECK FOR BOUNDS.
	/* Get the top cmd from UNDO_STACK and "undo" its action.
	 * Then, move it to the REDO_STACK. */
}

void redo(int times) {
	debug_printCmd(stack_pop(&REDO_STACK, &REDO_STACK_SIZE));
	/* Get the top cmd from REDO_STACK and "redo" its action.
	 * Then, move it to the UNDO_STACK. */
}