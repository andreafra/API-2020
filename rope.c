#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

typedef struct Node_ {
	char *value;
	struct Node_ *parent, *left, *right;
	unsigned size;
} Node;

typedef struct Tree_ {
	Node *root;
	unsigned size;
} Tree;

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

/* We need post-order binary tree traversal to free all nodes.
 * Input is a pointer to a tree.
 * This is a usual post-order binary tree traversal in which visit() conducts freeing a node.
 * This is iterative version, because it's several times faster than recursive.
 * A (little) speed-up can be gained by allocating stack statically instead of dynamically. */
/*void postOrderFree(Tree *tree) {
	unsigned n = tree->size;                                    // unsigned n = tree->size;
	Node **stack = malloc(n * sizeof(**stack));                  // stack contains pointers to nodes
	unsigned size = 0;											// current stack size
	char *boolStack = malloc(n * sizeof(*boolStack));           // For each element on the node stack, a corresponding value is stored on the bool stack. If this value is true, then we need to pop and visit the node on next encounter.
	unsigned boolSize = 0;
	char alreadyEncountered;                                    // boolean
	Node *current = tree->root;
	while (current) {
		stack[size++] = current;
		boolStack[boolSize++] = 0;                              // false
		current = current->left;
	}
	while (size) {
		current = stack[size - 1];
		alreadyEncountered = boolStack[boolSize - 1];
		if (alreadyEncountered) {
			free(current);                                      // visit()
			size--;
			boolSize--;
		}
		else {
			boolSize--;
			boolStack[boolSize++] = 1;                          // true
			current = current->right;
			while (current) {
				stack[size++] = current;
				boolStack[boolSize++] = 0;                      // false
				current = current->left;
			}
		}
	}
	tree->root = NULL;
	tree->size = 0;
	free(stack);
	free(boolStack);
}*/

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

/* Input: Pointer to a tree, and a pointer to its node object that we want to rotate right.
 * Returns nothing.
 * Doesn't splay any node. */
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

/* Input: Pointer to a tree, and a pointer to its node object that we want to rotate left.
 * Returns nothing.
 * Doesn't splay any node. */
static void rotateLeft(Tree *tree, Node *node) {
	Node *parent = node->parent;
	Node *X = node->right;
	if (!X)
		return;                                                 // we can't rotate the node with nothing!
	Node *B = X->left;
	X->parent = parent;
	if (parent) {
		if (node == parent->left)                               // node is left child
			parent->left = X;
		else                                                    // node is right child
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

/* Splays node to the top of the tree, making it new root of the tree.
 * Input: Pointer to a tree, and a pointer to its node object that we want to splay to the root.
 * Returns nothing. */
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


void PRINT_TREE(Tree *tree) {
	if (tree) {
		char **sstr = inOrder(tree);
		for (unsigned xx = 0; xx < tree->size; xx++) {
			printf("%s ", sstr[xx]);
		}
		printf("\n");
	} else printf("<null>\n");
}

// METHODS TO USE

/* Input: Integer number k - the rank of a node (0 <= k < size of the whole tree).
 * Output: The k - th smallest element in the tree (a node object). Counting starts from 0.
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

/* Input: rank is a numerical value (0 <= rank <= size of the whole tree); value is a lowercase English letter.
 * This is a general splay tree method, that works in general case.
 * Adds a node with letter "value" to the tree (string), at the position "rank". Numbering is 0-based.
 * Splays the node up to the top of the tree.
 * But, if we insert more than one character at one time (a string), it will also work. It will accept a string, and put it in a node.
 * Returns nothing.
 * Goes down from root to a leaf only once, and also goes up only once. */
/*static void Insert(Tree *tree, unsigned rank, char *value) {
	Node *node = createNode(value);

	// Inserting at the end of the whole text.
	if (rank == tree->size && tree->size > 0) {
		Node *last = Index(tree, rank - 1);
		node->left = last;
		node->size = last->size + 1;
		last->parent = node;
		tree->size++;
		tree->root = node;
		return;
	}

	// Inserting in the middle (or at the beginning).
	if (tree->size == 0) {
		// The tree is empty
		tree->size++;
		tree->root = node;
		return;
	}
	// This will be right node of the newly inserted node.
	Node *right = Index(tree, rank);
	node->right = right;
	node->left = right->left;
	right->parent = node;
	right->left = NULL;
	right->size = (right->right ? right->right->size : 0) + 1;
	node->size = (node->left ? node->left->size : 0) + (node->right ? node->right->size : 0) + 1;
	tree->size++;                                                       // Tree size
	tree->root = node;
}*/

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

/* This is cut-and-paste function.
 * For i and j, counting starts from 0; for k, counting starts from 1.
 * We paste the substring after the k - th symbol of the remaining string(after cutting).
 * If k == 0, we insert the substring at the beginning. */
/* We can't destroy left, middle and right at the end of the function,
 * because merge() doesn't create a new tree.
 * Function merge() returns a pointer to a tree, which means that its two parts
 * are preserved. It just merges them together.
 * Globally, we don't change the size of the tree in this function.
 * We just split it and then merge it back.
 * That's why total memory consumption is fine. */
void Process(Tree **tree, unsigned i, unsigned j, unsigned k) {
	Tree *left = NULL, *middle = NULL, *right = NULL;
	Split(*tree, j, &middle, &right);
	if (i > 0)
		Split(middle, i - 1, &left, &middle);
	left = Concat(left, right);
	if (k > 0)
		Split(left, k - 1, &left, &right);
	else {
		right = left;
		left = NULL;
	}
	*tree = Concat(Concat(left, middle), right);
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

int main() {
	Tree *tree = createTree();
	Insert(&tree, 0, "A");
	Insert(&tree, 1, "B");
	Insert(&tree, 2, "C");
	Insert(&tree, 3, "D");
	PRINT_TREE(tree);

	for (List *curr = Report(tree, 3,3); curr; curr = curr->next) {
		printf("%s ", curr->value->value);
	}

	destroyTree(tree);

	return 0;
}
